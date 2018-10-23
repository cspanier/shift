using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Threading;

namespace Shift.Network
{
    internal delegate void SocketEventHandler();

    public class BufferAvailableArgs : EventArgs
    {
    }

    public sealed class NetworkHost
    {
        // Private constructor to prevent direct instantiation.
        private NetworkHost()
        {
        }

        // Accessor for the esingleton instance.
        public static NetworkHost Instance
        {
            get
            {
                return s_instance.Value;
            }
        }

        public event EventHandler<BufferAvailableArgs> BufferAvailable;

        public List<IPAddress> LocalAddresses()
        {
            var result = new List<IPAddress>();
            foreach (var address in Dns.GetHostEntry(Dns.GetHostName()).AddressList)
            {
                /// ToDo: Only accept class A, B and C addresses.
                if (address.AddressFamily == AddressFamily.InterNetwork)
                    result.Add(address);
                else if (address.AddressFamily == AddressFamily.InterNetworkV6 && address.IsIPv6SiteLocal)
                    result.Add(address);
            }
            if (result.Count > 0)
                return result;
            throw new Exception("Local IP Address Not Found!");
        }

        public void Receive(TimeSpan minTimeout, TimeSpan? maxTimeout = null)
        {
            if (maxTimeout == null || minTimeout > maxTimeout)
                maxTimeout = minTimeout;
            var minEndTime = DateTime.Now.Add(minTimeout);
            var maxEndTime = DateTime.Now.Add((TimeSpan)maxTimeout);
            TimeSpan remainingTime = minTimeout;

            while (_eventCount.WaitOne(remainingTime))
            {
                SocketEventHandler socketEvent;
                if (!_eventQueue.TryDequeue(out socketEvent))
                    throw new InvalidOperationException();
                if (socketEvent != null)
                    socketEvent();

                var now = DateTime.Now;
                if (now < minEndTime)
                    remainingTime = minEndTime - now;
                else
                {
                    if (now < maxEndTime)
                        remainingTime = TimeSpan.Zero;
                    else
                        break;
                }
            }
        }

        internal void QueueEvent(SocketEventHandler socketEvent)
        {
            _eventQueue.Enqueue(socketEvent);
            _eventCount.Release();
            var bufferAvailable = BufferAvailable;
            if (bufferAvailable != null)
                bufferAvailable(this, new BufferAvailableArgs());
        }

        internal uint NextChannelId
        {
            get
            {
                return (uint)Interlocked.Increment(ref _nextChannelId);
            }
        }

        private static readonly Lazy<NetworkHost> s_instance =
            new Lazy<NetworkHost>(() => new NetworkHost());

        public bool DebugSerialization { get; set; }
        private int _nextChannelId;
        private ConcurrentQueue<SocketEventHandler> _eventQueue =
            new ConcurrentQueue<SocketEventHandler>();
        private Semaphore _eventCount = new Semaphore(0, Int32.MaxValue);
    }
}
