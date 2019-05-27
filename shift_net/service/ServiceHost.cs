using Shift.Network;
using System;
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Net.Sockets;

namespace Shift.Service
{
  public class ServiceHost
  {
    /// <summary>
    ///   A private constructor to prevent direct instantiation.
    /// </summary>
    private ServiceHost()
    {
      HostGuid = GuidGenerator.generate();
      MulticastPort = 0x700;
    }

    /// <summary>
    ///   The accessor for the esingleton instance.
    /// </summary>
    public static ServiceHost Instance
    {
      get
      {
        return s_instance.Value;
      }
    }

    public bool Running { get; private set; }

    public bool Start(IPAddress bindAddress, IPAddress multicastAddress4, IPAddress multicastAddress6,
      uint protocolVersion, uint requiredVersion)
    {
      if (Running)
        return false;
      BindAddress = bindAddress;
      MulticastAddress4 = multicastAddress4;
      MulticastAddress6 = multicastAddress6;
      ProtocolVersion = protocolVersion;
      RequiredVersion = requiredVersion;

      /// ToDo: Actually filter unspecified addresses like 0.0.0.0 or ::.
      if (bindAddress.AddressFamily == AddressFamily.InterNetwork ||
        bindAddress.AddressFamily == AddressFamily.InterNetworkV6)
      {
        var serviceSocket = new TcpListenerSocket();
        serviceSocket.Connected += OnClientConnected;
        serviceSocket.Disconnected += OnClientDisconnected;
        serviceSocket.Listen(new IPEndPoint(bindAddress, ServicePort));
        _serviceSockets.Add(serviceSocket);

        var multicastSocket = new UdpSocket();
        multicastSocket.MessageReceived += OnMulticastReceive;
        multicastSocket.Open(new IPEndPoint(bindAddress, MulticastPort),
                    (bindAddress.AddressFamily == AddressFamily.InterNetworkV6)
                    ? multicastAddress6
                    : multicastAddress4);
        _multicastSockets.Add(multicastSocket);
      }
      else
      {
        foreach (var address in NetworkHost.Instance.LocalAddresses())
        {
          var serviceSocket = new TcpListenerSocket();
          serviceSocket.Connected += OnClientConnected;
          serviceSocket.Disconnected += OnClientDisconnected;
          serviceSocket.Listen(new IPEndPoint(address, ServicePort));
          _serviceSockets.Add(serviceSocket);

          var multicastSocket = new UdpSocket();
          multicastSocket.MessageReceived += OnMulticastReceive;
          multicastSocket.Open(new IPEndPoint(address, MulticastPort),
                      (bindAddress.AddressFamily == AddressFamily.InterNetworkV6)
                      ? multicastAddress6
                      : multicastAddress4);
          _multicastSockets.Add(multicastSocket);
        }
      }
      Console.WriteLine(String.Format("Started ServiceHost {0:X}", HostGuid));
      Running = true;
      return Running;
    }

    public void Stop()
    {
      if (!Running)
        return;
      Running = false;
    }

    /// <summary>
    ///   Publish service information across the network. This method has to be
    ///   called on a regular basis to make different instances find each other.
    /// </summary>
    public void Publish()
    {
      var message = new MessageStream(1);
      lock (_serviceProviders)
      {
        var count = (ushort)Math.Min(_serviceProviders.Count, ushort.MaxValue);
        message.Write(HostGuid);
        message.Write(count);
        foreach (var provider in _serviceProviders)
        {
          message.Write(provider.ServiceGuid);
          message.Write(provider.ServiceUid);
          message.Write(provider.InterfaceUids);
          message.Write(provider.ServiceTag);
          message.Write(provider.ServicePort);
        }
      }
      foreach (var multicastSocket in _multicastSockets)
      {
        multicastSocket.PostTo(
          new IPEndPoint(multicastSocket.MulticastAddress, MulticastPort), message);
      }
    }

    /// <summary>
    ///   Event handler for receiving multicast messages send by other service
    ///   hosts.
    /// </summary>
    /// <param name="sender"></param>
    /// <param name="e"></param>
    private void OnMulticastReceive(object sender, UdpSocketReceiveEventArgs e)
    {
      try
      {
        ulong remoteHostGuid;
        ushort remoteServiceCount;
        e.Message.ProtocolVersion = 1;
        e.Message.Read(out remoteHostGuid);
        e.Message.Read(out remoteServiceCount);

        if (remoteHostGuid == HostGuid)
          return; // Dop own packet.

        if (remoteServiceCount == 0)
          return; // Nothing to do.

        if (DebugMulticasts)
        {
          Console.WriteLine(String.Format(
            "Received multicast message from {0:X} " +
            "containing {1} services.",
            remoteHostGuid, remoteServiceCount));
        }

        lock (_serviceClients)
        {
          for (; remoteServiceCount > 0; --remoteServiceCount)
          {
            ulong remoteServiceGuid;
            uint remoteServiceUid;
            ulong remoteInterfaceUids;
            ushort remoteServiceTag;
            ushort remoteServicePort;
            e.Message.Read(out remoteServiceGuid);
            e.Message.Read(out remoteServiceUid);
            e.Message.Read(out remoteInterfaceUids);
            e.Message.Read(out remoteServiceTag);
            e.Message.Read(out remoteServicePort);

            // Swap the two interface type ids contained in the
            // remote service type id to ease comparison with the
            // local service type ids.
            ulong localInterfaceUids =
              (remoteInterfaceUids << 32) |
              (remoteInterfaceUids >> 32);
            foreach (var serviceClient in _serviceClients)
            {
              if (remoteServiceUid == serviceClient.ServiceUid)
              {
                serviceClient.AddProvider(
                  remoteHostGuid, remoteServiceGuid,
                  remoteInterfaceUids, remoteServiceTag,
                  new IPEndPoint(e.RemoteEndPoint.Address, remoteServicePort));
              }
            }
          }
        }
      }
      catch (EndOfStreamException)
      {
      }
    }

    internal void RegisterClient(ServiceBase client)
    {
      lock (_serviceClients)
      {
        _serviceClients.Add(client);
      }
    }

    internal void DeregisterClient(ServiceBase client)
    {
      lock (_serviceClients)
      {
        _serviceClients.Remove(client);
      }
    }

    internal void RegisterProvider(ServiceBase provider)
    {
      provider.FoundIncompatibleRemoteService += FoundIncompatibleRemoteService;
      lock (_serviceProviders)
      {
        _serviceProviders.Add(provider);
      }
    }

    internal void DeregisterProvider(ServiceBase provider)
    {
      lock (_serviceProviders)
      {
        _serviceProviders.Remove(provider);
      }
      provider.FoundIncompatibleRemoteService -= FoundIncompatibleRemoteService;
    }

    private void OnClientConnected(object sender, Shift.Network.TcpSocketNotificationEventArgs e)
    {
    }

    private void OnClientDisconnected(object sender, TcpSocketNotificationEventArgs e)
    {
    }

    public ulong InterfaceId(Type type)
    {
      lock (_interfaces)
      {
        Tuple<uint, IDeserializer> pair;
        if (!_interfaces.TryGetValue(type, out pair))
        {
          // If you hit this exception you most likely forgot to register
          // your interface. The code generator should have written a
          // helper function for doing this.
          throw new ArgumentException("Type " + type.FullName +
            "is not a registered interface type.");
        }
        return pair.Item1;
      }
    }

    public IDeserializer InterfaceDeserializer(Type type)
    {
      lock (_interfaces)
      {
        Tuple<uint, IDeserializer> pair;
        if (!_interfaces.TryGetValue(type, out pair))
        {
          // If you hit this exception you most likely forgot to register
          // your interface. The code generator should have written a
          // helper function for doing this.
          throw new ArgumentException("Type " + type.FullName +
            "is not a registered interface type.");
        }
        return pair.Item2;
      }
    }

    /// <summary>
    ///   Generic version of InterfaceId(Type).
    /// </summary>
    /// <typeparam name="TInterface"></typeparam>
    /// <returns></returns>
    public ulong InterfaceId<TInterface>()
    {
      return InterfaceId(typeof(TInterface));
    }

    /// <summary>
    ///   Generic version of InterfaceDeserializer(Type).
    /// </summary>
    /// <typeparam name="TInterface"></typeparam>
    /// <returns></returns>
    public IDeserializer InterfaceDeserializer<TInterface>()
    {
      return InterfaceDeserializer(typeof(TInterface));
    }

    public void RegisterInterface<TInterface>(uint id, IDeserializer deserializer)
    {
      lock (_interfaces)
      {
        _interfaces.Add(typeof(TInterface),
          new Tuple<uint, IDeserializer>(id, deserializer));
      }
    }

    public event EventHandler<FoundIncompatibleRemoteServiceArgs> FoundIncompatibleRemoteService;

    public ulong HostGuid { get; set; }
    public ushort MulticastPort { get; private set; }
    public ushort ServicePort { get; private set; }
    public IPAddress BindAddress { get; private set; }
    public IPAddress MulticastAddress4 { get; private set; }
    public IPAddress MulticastAddress6 { get; private set; }
    public uint ProtocolVersion { get; private set; }
    public uint RequiredVersion { get; private set; }

    public bool DebugMulticasts { get; set; }
    public bool DebugConnections { get; set; }
    public bool DebugMessages { get; set; }
    public bool DebugSerializationPerformance { get; set; }
    public bool DebugMessagePerformance { get; set; }

    private static readonly Lazy<ServiceHost> s_instance =
      new Lazy<ServiceHost>(() => new ServiceHost());

    private Dictionary<Type, Tuple<uint, IDeserializer>> _interfaces =
      new Dictionary<Type, Tuple<uint, IDeserializer>>();
    private List<TcpListenerSocket> _serviceSockets = new List<TcpListenerSocket>();
    private List<UdpSocket> _multicastSockets = new List<UdpSocket>();

    private HashSet<ServiceBase> _serviceClients =
      new HashSet<ServiceBase>();
    private HashSet<ServiceBase> _serviceProviders =
      new HashSet<ServiceBase>();
  }
}
