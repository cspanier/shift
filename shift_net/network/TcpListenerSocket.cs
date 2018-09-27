using System;
using System.Diagnostics;
using System.Net;
using System.Net.Sockets;
using System.Threading.Tasks;

namespace Shift.Network
{
    public class TcpListenerSocket : TcpSocket
    {
        public TcpListenerSocket()
        {
        }

        public void Listen(IPEndPoint localEndPoint)
        {
            if (_listener != null)
                throw new InvalidOperationException("You cannot listen on a Tcp socket twice.");
            _listener = new TcpListener(localEndPoint);
            _listener.Start();

            Task.Factory.StartNew(async () =>
            {
                try
                {
                    while (true)
                    {
                        var client = new TcpClientSocket(this,
                            await _listener.AcceptTcpClientAsync());
                    }
                }
                catch (InvalidOperationException)
                {
                    // The listener has not been started with a call to Start.
                    throw;
                }
                catch (SocketException ex)
                {
                    Debug.Write(ex);
                    Close();
                }
            });
        }

        public override bool Close()
        {
            if (_listener == null)
                return true;
            _listener.Stop();
            _listener = null;
            return true;
        }

        public IPEndPoint LocalEndPoint { get { return (IPEndPoint)_listener.LocalEndpoint; } }

        private TcpListener _listener;
    }
}
