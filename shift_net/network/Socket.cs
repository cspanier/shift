using System;
using System.Net;

namespace Shift.Network
{
  public class TcpSocketNotificationEventArgs : EventArgs
  {
    public TcpSocketNotificationEventArgs(TcpClientSocket socket, int errorCode = 0)
    {
      Socket = socket;
      ErrorCode = errorCode;
    }

    public TcpClientSocket Socket { get; private set; }
    public int ErrorCode { get; private set; }
  }

  public class TcpSocketReceiveEventArgs : EventArgs
  {
    public TcpSocketReceiveEventArgs(TcpClientSocket socket, MessageStream message)
    {
      Socket = socket;
      Stream = message;
    }

    public TcpClientSocket Socket { get; private set; }
    public MessageStream Stream { get; private set; }
  }

  public class UdpSocketReceiveEventArgs : EventArgs
  {
    public UdpSocketReceiveEventArgs(UdpSocket socket, IPEndPoint remoteEndPoint,
      MessageStream message)
    {
      Socket = socket;
      RemoteEndPoint = remoteEndPoint;
      Message = message;
    }

    public UdpSocket Socket { get; private set; }
    public IPEndPoint RemoteEndPoint { get; private set; }
    public MessageStream Message { get; private set; }
  }

  public abstract class Socket
  {
    /// <summary>
    ///   Default constructor.
    /// </summary>
    public Socket()
    {
      ChannelId = NetworkHost.Instance.NextChannelId;
    }

    public MessageFactory Factory { get; set; }

    /// <summary>
    ///   Grants access to the associated channel id.
    /// </summary>
    public uint ChannelId { get; internal set; }

    /// <summary>
    ///   Closes the socket.
    /// </summary>
    /// <returns>Returns true if the socket has been closed, or false if it has already been
    /// closed.</returns>
    public abstract bool Close();

    /// <summary>
    ///   Sends the argument message to the connected remote end-point.
    /// </summary>
    /// <param name="message">The message buffer to send.</param>
    /// <remarks>Only valid for Tcp client sockets.</remarks>
    public virtual void Post(MessageStream message)
    {
      throw new NotImplementedException();
    }

    /// <summary>
    ///   Sends the argument message to the specified remote end-point.
    /// </summary>
    /// <param name="destination">The remote address and port to send the buffer to.</param>
    /// <param name="message">The message buffer to send.</param>
    public virtual void PostTo(IPEndPoint destination, MessageStream message)
    {
      throw new NotImplementedException();
    }

    /// <summary>
    ///   Pushes a buffer to the internal queue. This method may be used to
    ///   manually inject data as if it was received through this socket.
    /// </summary>
    /// <param name="message">The message buffer to send.</param>
    /// <remarks>Only valid for Tcp client sockets.</remarks>
    public virtual void Queue(MessageStream message)
    {
      throw new NotImplementedException();
    }

    /// <summary>
    ///   Pushes a buffer to the internal queue. This method may be used to
    ///   manually inject data as if it was received through this socket.
    /// </summary>
    /// <param name="remoteEndPoint"></param>
    /// <param name="message"></param>
    /// <remarks>Only valid for Udp sockets.</remarks>
    public virtual void Queue(IPEndPoint remoteEndPoint, MessageStream message)
    {
      throw new NotImplementedException();
    }
  }
}
