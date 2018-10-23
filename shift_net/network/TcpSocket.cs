using System;

namespace Shift.Network
{
    public abstract class TcpSocket : Socket
    {
        /// <summary>
        /// An internal helper method to indirectly call the Connected event from outside.
        /// </summary>
        internal void OnConnected(TcpSocketNotificationEventArgs e)
        {
            var handler = Connected;
            if (handler != null)
            {
                NetworkHost.Instance.QueueEvent(() =>
                {
                    handler(this, e);
                    // Enter the read queue only after the connected event has been processed.
                    e.Socket.StartReadTask();
                });
            }
            else
                e.Socket.StartReadTask();
        }

        /// <summary>
        /// An internal helper method to indirectly call the Disconnected event from outside.
        /// </summary>
        internal void OnDisconnected(TcpSocketNotificationEventArgs e)
        {
            var handler = Disconnected;
            if (handler != null)
            {
                NetworkHost.Instance.QueueEvent(() =>
                {
                    handler(this, e);
                });
            }
        }

        public event EventHandler<TcpSocketNotificationEventArgs> Connected;
        public event EventHandler<TcpSocketNotificationEventArgs> Disconnected;
    }
}
