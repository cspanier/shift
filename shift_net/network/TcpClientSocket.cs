using System;
using System.Collections.Concurrent;
using System.Diagnostics;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Threading.Tasks;
#if WINDOWS_UWP
using Windows.Storage.Streams;
using System.Runtime.InteropServices.WindowsRuntime;
#endif

namespace Shift.Network
{
    public class TcpClientSocket : TcpSocket
    {
        /// <summary>
        /// Constructor to create a new Tcp client socket.
        /// </summary>
        public TcpClientSocket()
        {
        }

        /// <summary>
        /// Internal constructor used to create a Tcp client socket from an existing socket.
        /// </summary>
        internal TcpClientSocket(TcpListenerSocket server, TcpClient client)
        {
            _parent = server;
            _tcpSocket = client;
            _tcpSocket.NoDelay = true;
            _stream = _tcpSocket.GetStream();
            _parent.OnConnected(new TcpSocketNotificationEventArgs(this));
            StartWriteTask();
        }

        public void Connect(IPEndPoint remoteEndPoint)
        {
            if (_tcpSocket != null)
                throw new InvalidOperationException("You cannot connect a Tcp client socket twice.");
            _tcpSocket = new TcpClient();
            _tcpSocket.NoDelay = true;
            Task.Factory.StartNew(async () =>
            {
                try
                {
                    await _tcpSocket.ConnectAsync(remoteEndPoint.Address, remoteEndPoint.Port);
                    _stream = _tcpSocket.GetStream();
                    OnConnected(new TcpSocketNotificationEventArgs(this));
                    StartWriteTask();
                }
                catch (ObjectDisposedException)
                {
                    // The underlying Socket has been closed.
                    OnConnectFailed(new TcpSocketNotificationEventArgs(this));
                    Close();
                }
                catch (Exception ex)
                {
                    Debug.Write(ex);
                    OnConnectFailed(new TcpSocketNotificationEventArgs(this));
                    Close();
                }
            });
        }

        public override bool Close()
        {
            if (_tcpSocket == null)
                return true;
            OnDisconnected(new TcpSocketNotificationEventArgs(this, 0));
            if (_parent != null)
            {
                // Also tell the parent that this socket disconnected.
                _parent.OnDisconnected(new TcpSocketNotificationEventArgs(this, 0));
            }
            if (_stream != null)
                _stream = null;
            if (_tcpSocket != null)
            {
                _tcpSocket.Close();
                _tcpSocket = null;
            }
            _quit.Cancel();
            return true;
        }

        /// <summary>
        ///     Pushes the argument to the queue of buffers that are to be written
        ///     asynchronously to the data stream.
        /// </summary>
        /// <param name="message">The message buffer to send.</param>
        /// <see cref="Shift.Network.Socket.Post"/>
        public override void Post(MessageStream message)
        {
            if (_tcpSocket == null)
                return;
            _sendQueue.Enqueue(message);
            _sendCount.Release();
        }

        /// <summary>
        ///     Pushes a buffer to the internal queue. This method may be used to
        ///     manually inject data as if it was received through this socket.
        /// </summary>
        /// <param name="message">The message buffer to send.</param>
        /// <see cref="Shift.Network.Socket.Queue"/>
        public override void Queue(MessageStream message)
        {
            NetworkHost.Instance.QueueEvent(() =>
            {
                var handler = MessageReceived;
                if (handler != null)
                    handler(this, new TcpSocketReceiveEventArgs(this, message));
            });
        }

        /// <summary>
        ///     Grants access to the parent socket, if there is one.
        /// </summary>
        public TcpListenerSocket Parent
        {
            get { return _parent; }
            protected set { _parent = value; }
        }

        /// <summary>
        /// Enters the read loop.
        /// </summary>
        internal void StartReadTask()
        {
            Task.Factory.StartNew(async () =>
            {
                try
                {
#if WINDOWS_UWP
                    IBuffer lengthBuffer;
#else
                    var lengthBuffer = new byte[sizeof(uint)];
#endif
                    while (true)
                    {
#if WINDOWS_UWP
                        lengthBuffer = new Windows.Storage.Streams.Buffer(sizeof(uint));
                        while (lengthBuffer.Length < sizeof(uint))
                        {
                            var progress = await _stream.ReadAsync(lengthBuffer, lengthBuffer.Capacity - lengthBuffer.Length, InputStreamOptions.Partial);
                            if (progress == null)
                            {
                                Close();
                                return;
                            }
                        }

                        /// ToDo: Check BitConverter.IsLittleEndian.
                        var messageLength = BitConverter.ToUInt32(lengthBuffer.ToArray(), 0);
                        if (messageLength <= 0)
                            continue;
                        var messageBuffer = new Windows.Storage.Streams.Buffer(messageLength);
                        while (messageBuffer.Length < messageLength)
                        {
                            var progress = await _stream.ReadAsync(messageBuffer, messageBuffer.Capacity - messageBuffer.Length, InputStreamOptions.Partial);
                            if (progress == null)
                            {
                                Close();
                                return;
                            }
                        }

                        NetworkHost.Instance.QueueEvent(() =>
                        {
                            var handler = MessageReceived;
                            if (handler != null)
                            {
                                var message = new MessageStream(messageBuffer.ToArray());
                                handler(this, new TcpSocketReceiveEventArgs(this, message));
                            }
                        });
#else
                        var bufferPosition = 0;
                        while (bufferPosition < sizeof(uint))
                        {
                            var progress = await _stream.ReadAsync(lengthBuffer,
                                bufferPosition, lengthBuffer.Length - bufferPosition, _quit.Token);
                            if (progress == 0)
                            {
                                Close();
                                return;
                            }
                            bufferPosition += progress;
                        }

                        bufferPosition = 0;
                        /// ToDo: Check BitConverter.IsLittleEndian.
                        var messageLength = BitConverter.ToUInt32(lengthBuffer, 0);
                        if (messageLength <= 0)
                            continue;
                        var messageBuffer = new byte[messageLength];
                        while (bufferPosition < messageLength)
                        {
                            var progress = await _stream.ReadAsync(messageBuffer,
                                bufferPosition, messageBuffer.Length - bufferPosition, _quit.Token);
                            if (progress == 0)
                            {
                                Close();
                                return;
                            }
                            bufferPosition += progress;
                        }
                        NetworkHost.Instance.QueueEvent(() =>
                            {
                                var handler = MessageReceived;
                                if (handler != null)
                                {
                                    var message = new MessageStream(messageBuffer);
                                    handler(this, new TcpSocketReceiveEventArgs(this, message));
                                }
                            });
#endif
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

        internal void StartWriteTask()
        {
            Task.Factory.StartNew(async () =>
            {
                try
                {
                    while (true)
                    {
                        await _sendCount.WaitAsync(_quit.Token);
                        MessageStream message;
                        if (!_sendQueue.TryDequeue(out message))
                            break;
                        // Copy _stream to a local variable and test it for null because Close()
                        // might reset it in parallel.
                        var stream = _stream;
                        if (message != null && stream != null)
                        {
                            /// ToDo: It would be nice if each message would reserve an
                            /// uint to store its length at the beginning of each buffer
                            /// to save the additional copy.
                            uint messageLength = (uint)message.Length;
                            var sendBuffer = new byte[message.Length + sizeof(uint)];
                            BitConverter.GetBytes(messageLength).CopyTo(sendBuffer, 0);
                            Array.Copy(message.Buffer, 0, sendBuffer, sizeof(uint), (int)message.Length);
#if WINDOWS_UWP
                            await stream.WriteAsync(sendBuffer.AsBuffer());
#else
                            await stream.WriteAsync(sendBuffer, 0, sendBuffer.Length, _quit.Token);
#endif
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

        /// <summary>
        /// An internal helper method to indirectly call the ConnectFailed event from outside.
        /// </summary>
        internal void OnConnectFailed(TcpSocketNotificationEventArgs e)
        {
            var handler = ConnectFailed;
            if (handler != null)
            {
                NetworkHost.Instance.QueueEvent(() =>
                {
                    handler(this, e);
                });
            }
        }

        public IPEndPoint LocalEndPoint { get { return (IPEndPoint)_tcpSocket.Client.LocalEndPoint; } }
        public IPEndPoint RemoteEndPoint { get { return (IPEndPoint)_tcpSocket.Client.RemoteEndPoint; } }

        public event EventHandler<TcpSocketNotificationEventArgs> ConnectFailed;
        public event EventHandler<TcpSocketReceiveEventArgs> MessageReceived;

        private TcpListenerSocket _parent;
        private TcpClient _tcpSocket;
        private NetworkStream _stream;

        private ConcurrentQueue<MessageStream> _sendQueue =
            new ConcurrentQueue<MessageStream>();
        private SemaphoreSlim _sendCount = new SemaphoreSlim(0, Int32.MaxValue);
        private CancellationTokenSource _quit = new CancellationTokenSource();
    }
}
