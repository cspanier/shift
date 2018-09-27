using Shift.Network;
using System;
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Threading;

namespace Shift.Service
{
    public class FoundRemoteServiceArgs : EventArgs
    {
        public FoundRemoteServiceArgs(ulong hostGuid, ulong serviceGuid,
            ushort serviceTag, IPEndPoint remoteEndPoint)
        {
            HostGuid = hostGuid;
            ServiceGuid = serviceGuid;
            ServiceTag = serviceTag;
            RemoteEndPoint = remoteEndPoint;
            Accept = false;
        }

        public ulong HostGuid { get; private set; }
        public ulong ServiceGuid { get; private set; }
        public ushort ServiceTag { get; private set; }
        public IPEndPoint RemoteEndPoint { get; private set; }

        /// <summary>
        ///     The accept property serves as a result state which is modified
        ///     by the methods registered at the event.
        /// </summary>
        public bool Accept { get; set; }
    };

    public class FoundIncompatibleRemoteServiceArgs : EventArgs
    {
        public FoundIncompatibleRemoteServiceArgs(ulong remoteHostGuid, ulong serviceGuid, ulong remoteServiceGuid,
            uint serviceUid, ulong iterfaceUids, ulong remoteInterfaceUids, IPEndPoint remoteEndPoint)
        {
            RemoteHostGuid = remoteHostGuid;
            ServiceGuid = serviceGuid;
            RemoteServiceGuid = remoteServiceGuid;
            ServiceUid = serviceUid;
            InterfaceUids = iterfaceUids;
            RemoteInterfaceUids = remoteInterfaceUids;
            RemoteEndPoint = remoteEndPoint;
        }

        public ulong RemoteHostGuid { get; private set; }
        public ulong ServiceGuid { get; private set; }
        public ulong RemoteServiceGuid { get; private set; }
        public uint ServiceUid { get; private set; }
        public ulong InterfaceUids { get; private set; }
        public ulong RemoteInterfaceUids { get; private set; }
        public IPEndPoint RemoteEndPoint { get; private set; }
    }

    public class ConnectedRemoteServiceArgs : EventArgs
    {
        public ConnectedRemoteServiceArgs(ulong hostGuid, ulong serviceGuid,
            ushort serviceTag, IPEndPoint remoteEndPoint)
        {
            HostGuid = hostGuid;
            ServiceGuid = serviceGuid;
            ServiceTag = serviceTag;
            RemoteEndPoint = remoteEndPoint;
        }

        public ulong HostGuid { get; private set; }
        public ulong ServiceGuid { get; private set; }
        public ushort ServiceTag { get; private set; }
        public IPEndPoint RemoteEndPoint { get; private set; }
    };

    public class DisconnectedRemoteServiceArgs : EventArgs
    {
        public DisconnectedRemoteServiceArgs(ulong hostGuid, ulong serviceGuid)
        {
            HostGuid = hostGuid;
            ServiceGuid = serviceGuid;
        }

        public ulong HostGuid { get; private set; }
        public ulong ServiceGuid { get; private set; }
    };

    public class ServiceBase
    {
        public event EventHandler<FoundRemoteServiceArgs> FoundRemoteService;
        public event EventHandler<FoundIncompatibleRemoteServiceArgs> FoundIncompatibleRemoteService;
        public event EventHandler<ConnectedRemoteServiceArgs> ConnectedRemoteService;
        public event EventHandler<DisconnectedRemoteServiceArgs> DisconnectedRemoteService;

        public ServiceBase(bool isProvider, uint serviceUid, ulong interfaceUids, ushort serviceTag)
        {
            IsProvider = isProvider;
            ServiceGuid = GuidGenerator.generate();
            ServiceUid = serviceUid;
            InterfaceUids = interfaceUids;
            ServiceTag = serviceTag;
            Sockets = new HashSet<TcpClientSocket>();
            if (IsProvider)
            {
                ListenerSocket = new TcpListenerSocket();
                ListenerSocket.Connected += OnConnected;
                ListenerSocket.Disconnected += OnDisconnected;
                ListenerSocket.Listen(new IPEndPoint(
                    ServiceHost.Instance.BindAddress, 0));
            }
            Caller = new ThreadLocal<Connection>(
                () =>
                {
                    throw new NotImplementedException();
                }, false);
        }

        public uint ConnectedServicesCount
        {
            get { return _connectedServicesCount; }
        }

        /// <summary>
        ///     Depending on the value of IsProvider this either starts searching for
        ///     service providers or adds the service to the list of provided services.
        /// </summary>
        public void Start()
        {
            if (IsStarted)
                return;

            if (IsProvider)
                ServiceHost.Instance.RegisterProvider(this);
            else
                ServiceHost.Instance.RegisterClient(this);
            IsStarted = true;
        }

        /// <summary>
        ///     Stops searching for service providers or clients.
        /// </summary>
        public void Stop()
        {
            if (!IsStarted)
                return;

            if (IsProvider)
                ServiceHost.Instance.DeregisterProvider(this);
            else
                ServiceHost.Instance.DeregisterClient(this);
            IsStarted = false;
        }

        /// <summary>
        ///     This method gets called by the ServiceHost each time a service provider
        ///     with a matching serviceTypeId was found. Naturally this gets only called
        ///     for service clients.
        /// </summary>
        /// <param name="remoteHostGuid"></param>
        /// <param name="serviceGuid"></param>
        /// <param name="serviceTag"></param>
        /// <param name="remoteEndPoint"></param>
        internal bool AddProvider(ulong remoteHostGuid, ulong remoteServiceGuid,
            ulong remoteInterfaceUids, ushort serviceTag, IPEndPoint remoteEndPoint)
        {
            if (IsProvider)
                throw new InvalidOperationException();

            lock (_knownRemoteServiceGuids)
            {
                // Swap the two interface uids to ease comparison with the
                // local uids.
                remoteInterfaceUids =
                    (remoteInterfaceUids << 32) |
                    (remoteInterfaceUids >> 32);

                if (_knownRemoteServiceGuids.Contains(remoteServiceGuid))
                    return false;
                _knownRemoteServiceGuids.Add(remoteServiceGuid);

                if (remoteInterfaceUids != InterfaceUids)
                {
                    var foundIncompatibleRemoteService = FoundIncompatibleRemoteService;
                    if (foundIncompatibleRemoteService != null)
                    {
                        var e = new FoundIncompatibleRemoteServiceArgs(
                            remoteHostGuid, ServiceGuid, remoteServiceGuid,
                            ServiceUid, InterfaceUids, remoteInterfaceUids,
                            remoteEndPoint);
                        foundIncompatibleRemoteService(this, e);
                    }
                    return false;
                }

                lock (_connections)
                {
                    foreach (var connection in _connections)
                        if (connection.Value.ServiceGuid == remoteServiceGuid)
                            throw new InvalidOperationException();

                    bool accept = true;
                    var foundRemoteService = FoundRemoteService;
                    if (foundRemoteService != null)
                    {
                        var e = new FoundRemoteServiceArgs(remoteHostGuid, remoteServiceGuid,
                            serviceTag, remoteEndPoint);
                        foundRemoteService(this, e);
                        accept = e.Accept;
                    }
                    if (!accept)
                        return false;

                    var socket = new TcpClientSocket();
                    socket.ConnectFailed += OnConnectFailed;
                    socket.Connected += OnConnected;
                    socket.Disconnected += OnDisconnected;
                    socket.Connect(remoteEndPoint);
                    return true;
                }
            }
        }

        /// <summary>
        ///     This method is called by the ServiceHost each time a service
        ///     client connected and requested to talk to this service provider.
        /// </summary>
        /// <param name="remoteHostGuid"></param>
        /// <param name="remoteServiceGuid"></param>
        /// <param name="socket"></param>
        internal void AddClient(ulong remoteHostGuid, ulong remoteServiceGuid,
            Socket socket)
        {
        }

        /// <summary>
        ///     The attempt to connect to the remote service has failed for whatever
        ///     reason.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnConnectFailed(Object sender, TcpSocketNotificationEventArgs e)
        {
            // This may happen if the remote service died right after broadcasting
            // its address.
            /// ToDo: Print out some log message.
            Console.WriteLine(String.Format("Warning: Connection #{0} failed.",
                e.Socket.ChannelId));
        }

        /// <summary>
        ///     This method gets called when the connection to a service has been
        ///     established.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        /// <remarks>
        ///   The method sends the initial handshake message, which is used to
        ///   validate that the two services actually match (using their
        ///   serviceTypeId, serviceGuids and hostGuids) and to agree on a
        ///   protocol version supported by both ends. The answer will be received
        ///   using onReceiveHandshake. If the remote service rejects the connect
        ///   attempt it will close the connection and thus result in a call to
        ///   onDisconnected.
        /// </remarks>
        private void OnConnected(Object sender, TcpSocketNotificationEventArgs e)
        {
            e.Socket.MessageReceived += OnReceiveHandshake;
            if (!IsProvider)
                SendHandshake(e.Socket);

            if (ServiceHost.Instance.DebugConnections)
            {
                Console.WriteLine(String.Format("Established connection #{0} to remote service.",
                    e.Socket.ChannelId));
            }
        }

        /// <summary>
        ///     This method gets called when the remote service provider closes the
        ///     connection.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnDisconnected(Object sender, TcpSocketNotificationEventArgs e)
        {
            lock (_connections)
            {
                if (ServiceHost.Instance.DebugConnections)
                {
                    Console.WriteLine(String.Format("Lost connection #{0} to remote service.",
                        e.Socket.ChannelId));
                }

                Connection connection;
                if (!_connections.TryGetValue(e.Socket.ChannelId, out connection))
                {
                    Console.WriteLine(String.Format("Error: Failed to remove connection #{0}.",
                        e.Socket.ChannelId));
                    return;
                }
                lock (Sockets)
                {
                    Sockets.Remove(e.Socket);
                }
                --_connectedServicesCount;

                var handler = DisconnectedRemoteService;
                if (handler != null)
                {
                    handler(this, new DisconnectedRemoteServiceArgs(
                        connection.HostGuid, connection.ServiceGuid));
                }

                _connections.Remove(e.Socket.ChannelId);
            }
        }

        /// <summary>
        ///     This method gets called for the first received message, which is the
        ///     handshake.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        /// <remarks>
        ///     Once the handshake is received and accepted, the onReceive event is
        ///     re-bound to ServiceBase::onReceive.
        /// </remarks>
        private void OnReceiveHandshake(Object sender, TcpSocketReceiveEventArgs e)
        {
            try
            {
                uint protocolVersion;
                ulong hostGuid;
                ulong serviceGuid;
                uint serviceUid;
                ulong interfaceUids;
                e.Stream.ProtocolVersion = 1;
                e.Stream.Read(out protocolVersion);
                e.Stream.Read(out hostGuid);
                e.Stream.Read(out serviceGuid);
                e.Stream.Read(out serviceUid);
                e.Stream.Read(out interfaceUids);

                if (ServiceUid != serviceUid)
                    throw new InvalidDataException("The service's Uid do not match.");
                if (InterfaceUids != (interfaceUids << 32 | interfaceUids >> 32))
                    throw new InvalidDataException("The service interfaces' Uids do not match.");

                var connection = new Connection(this, e.Socket);
                connection.ProtocolVersion = protocolVersion;
                connection.HostGuid = hostGuid;
                connection.ServiceGuid = serviceGuid;
                connection.ServiceUid = serviceUid;
                connection.InterfaceUids = interfaceUids;

                e.Socket.MessageReceived -= OnReceiveHandshake;
                e.Socket.MessageReceived += OnReceiveMessage;

                if (IsProvider)
                    SendHandshake(e.Socket);

                lock (_connections)
                {
                    _connections.Add(e.Socket.ChannelId, connection);
                    ++_connectedServicesCount;
                    lock (Sockets)
                    {
                        Sockets.Add(e.Socket);
                    }
                    var connectedRemoteService = ConnectedRemoteService;
                    if (connectedRemoteService != null)
                    {
                        connectedRemoteService(this,
                            new ConnectedRemoteServiceArgs(
                                connection.HostGuid, connection.ServiceGuid,
                                connection.ServiceTag, connection.Socket.RemoteEndPoint));
                    }
                }
            }
            catch
            {
                e.Socket.Close();
            }
        }

        private void OnReceiveMessage(Object sender, TcpSocketReceiveEventArgs e)
        {
            lock (_connections)
            {
                Connection connection;
                if (!_connections.TryGetValue(e.Socket.ChannelId, out connection))
                    throw new InvalidOperationException();

                e.Stream.ProtocolVersion = connection.ProtocolVersion;
                Caller.Value = connection;
                Deserializer.Receive(e.Stream, _implementation);
                Caller.Value = null;
            }
        }

        private void SendHandshake(TcpClientSocket socket)
        {
            var handshake = new MessageStream(1, 5 * sizeof(uint));
            handshake.Write(ServiceHost.Instance.ProtocolVersion);
            handshake.Write(ServiceHost.Instance.HostGuid);
            handshake.Write(ServiceGuid);
            handshake.Write(ServiceUid);
            handshake.Write(InterfaceUids);
            socket.Post(handshake);
        }

        /// <summary>
        ///     Send a message to the specified remote service.
        /// </summary>
        public void SendTo(ulong remoteServiceGuid, Message message)
        {
            if (message == null)
                throw new ArgumentNullException();

            /// ToDo: Which protocol version to use?

            lock (_connections)
            {
                foreach (var connection in _connections)
                {
                    if (connection.Value.ServiceGuid == remoteServiceGuid)
                    {
                        var stream = new Shift.Network.MessageStream(1);
                        stream.Write(message.MessageUid);
                        stream.Write(message, null, new Group());
                        connection.Value.Socket.Post(stream);
                        return;
                    }
                }
                Console.WriteLine("Warning: Attempt to send message to the remote service " +
                                  remoteServiceGuid + ", which does not exist.");
            }
        }

        public bool IsProvider { get; private set; }
        public bool IsStarted { get; private set; }
        public ulong ServiceGuid { get; private set; }
        public uint ServiceUid { get; private set; }
        public ulong InterfaceUids { get; private set; }
        public ushort ServiceTag { get; protected set; }
        public ushort ServicePort
        {
            get
            {
                if (!IsProvider)
                {
                    throw new InvalidOperationException(
                        "Service clients do not have a service port.");
                }
                return (ushort)ListenerSocket.LocalEndPoint.Port;
            }
        }

        protected HashSet<TcpClientSocket> Sockets { get; private set; }
        protected TcpListenerSocket ListenerSocket { get; private set; }
        protected object _implementation = null;
        protected IDeserializer Deserializer { get; set; }
        private uint _connectedServicesCount;
        internal ThreadLocal<Connection> Caller { get; private set; }

        private SortedSet<ulong> _knownRemoteServiceGuids =
            new SortedSet<ulong>();
        private Dictionary<uint, Connection> _connections =
            new Dictionary<uint, Connection>();
        //private ulong _bytesSend = 0;
        //private ulong _bytesReceived = 0;
        //private ulong _bytesSentPerSecond = 0;
        //private ulong _bytesReceivedPerSecond = 0;
        //private ulong _packetsSent = 0;
        //private ulong _packetsReceived = 0;
        //private float _packetsSentPerSecond = 0.0f;
        //private float _packetsReceivedPerSecond = 0.0f;
    }
}
