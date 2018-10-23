using System;
using System.Collections.Concurrent;
using System.Diagnostics;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Threading.Tasks;

namespace Shift.Network
{
    public sealed class UdpSocket : Socket
    {
        public UdpSocket()
        {
        }

        public void Open(IPEndPoint localEndPoint, IPAddress multicastAddress)
        {
            if (_client != null)
                throw new InvalidOperationException("You cannot open an Udp socket twice.");
            LocalEndPoint = localEndPoint;
            MulticastAddress = multicastAddress;

            _client = new UdpClient(LocalEndPoint.AddressFamily);
            _client.Client.SetSocketOption(SocketOptionLevel.Socket,
                SocketOptionName.ReuseAddress, true);
            _client.Client.Bind(LocalEndPoint);
            if (MulticastAddress != null)
            {
                // Even though we previously bound to a specific address we still 
                // need to tell JoinMulticastGroup where to send the join message.
                _client.JoinMulticastGroup(MulticastAddress, localEndPoint.Address);
            }

            // When setting MulticastLoopback to true the socket received its own messages.
            // However, when setting it to false, it does not receive multicast messages from
            // another application on the local machine, so we have to set it to true.
            _client.MulticastLoopback = true;

            StartReadTask();
            StartWriteTask();
        }

        public override bool Close()
        {
            if (_client != null)
            {
                _client.Close();
                _client = null;
            }
            _quit.Cancel();
            return true;
        }

        /// <summary>
        ///     Pushes the argument to the queue of buffers that are to be written
        ///     asynchronously to the datagram socket.
        /// </summary>
        /// <param name="destination">The remote address and port to send the buffer to.</param>
        /// <param name="stream">The message buffer to send.</param>
        /// <see cref="Shift.Network.Socket.PostTo"/>
        public override void PostTo(IPEndPoint destination, MessageStream message)
        {
            if (_client == null)
                throw new InvalidOperationException();
            if (message.Length > ushort.MaxValue)
                throw new ArgumentException("Message too large", "message");
            _sendQueue.Enqueue(new Tuple<IPEndPoint, MessageStream>(destination, message));
            _sendCount.Release();
        }

        /// <summary>
        ///     Pushes a buffer to the internal queue. This method may be used to
        ///     manually inject data as if it was received through this socket.
        /// </summary>
        /// <param name="remoteEndPoint"></param>
        /// <param name="message"></param>
        /// <see cref="Shift.Network.Socket.Queue"/>
        public override void Queue(IPEndPoint remoteEndPoint, MessageStream message)
        {
            NetworkHost.Instance.QueueEvent(() =>
            {
                if (MessageReceived != null)
                    MessageReceived(this, new UdpSocketReceiveEventArgs(this, remoteEndPoint, message));
            });
        }

        /// <summary>
        ///     Enters the read loop.
        /// </summary>
        private void StartReadTask()
        {
            Task.Factory.StartNew(async () =>
            {
                while (true)
                {
                    try
                    {
                        var datagram = await _client.ReceiveAsync();
                        NetworkHost.Instance.QueueEvent(() =>
                            {
                                if (MessageReceived != null)
                                {
                                    MessageReceived(this, new UdpSocketReceiveEventArgs(this,
                                        (IPEndPoint)datagram.RemoteEndPoint,
                                        new MessageStream(datagram.Buffer)));
                                }
                            });
                    }
                    catch (ObjectDisposedException)
                    {
                        // The underlying Socket has been closed.
                        Close();
                        break;
                    }
                    catch (SocketException ex)
                    {
                        Debug.Write(ex);
                        Close();
                        break;
                    }
                }
            }, _quit.Token);
        }

        internal void StartWriteTask()
        {
            Task.Factory.StartNew(async () =>
            {
                try
                {
                    while (true)
                    {
                        await _sendCount.WaitAsync(_quit.Token);
                        Tuple<IPEndPoint, MessageStream> message;
                        if (!_sendQueue.TryDequeue(out message))
                            break;
                        // Copy _client to a local variable and test it for null because Close()
                        // might reset it in parallel.
                        var client = _client;
                        if (message != null && client != null)
                        {
                            var destination = message.Item1;
                            var stream = message.Item2;
                            await client.SendAsync(stream.Buffer, (int)stream.Length, destination);
                        }
                    }
                }
                catch (ObjectDisposedException)
                {
                    // The underlying Socket has been closed.
                    Close();
                }
                catch (Exception ex)
                {
                    Debug.Write(ex);
                    Close();
                }
            }, _quit.Token);
        }

        public event EventHandler<UdpSocketReceiveEventArgs> MessageReceived;
        public IPEndPoint LocalEndPoint { get; private set; }
        public IPAddress MulticastAddress { get; private set; }

        private UdpClient _client;

        private ConcurrentQueue<Tuple<IPEndPoint, MessageStream>> _sendQueue =
            new ConcurrentQueue<Tuple<IPEndPoint, MessageStream>>();
        private SemaphoreSlim _sendCount = new SemaphoreSlim(0, Int32.MaxValue);
        private CancellationTokenSource _quit = new CancellationTokenSource();
    }
}
