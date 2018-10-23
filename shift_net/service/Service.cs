using Shift.Network;
using System;

namespace Shift.Service
{
    public class InterfaceInfo<TServiceInterface>
    {
    };

    public class Service<TRemoteInterface, TLocalInterface> : ServiceBase
    {
        /// <summary>
        ///     Constructor.
        /// </summary>
        /// <param name="isProvider"></param>
        /// <param name="serviceUid"></param>
        /// <param name="serviceTag"></param>
        public Service(bool isProvider, uint serviceUid, ushort serviceTag)
            : base(isProvider, serviceUid,
                (ulong)ServiceHost.Instance.InterfaceId<TRemoteInterface>() |
                (ulong)ServiceHost.Instance.InterfaceId<TLocalInterface>() << 32,
                serviceTag)
        {
        }

        /// <summary>
        ///     Returns the Guid of the service which called the last one of this
        ///     service's routines. This call is only valid from within one of the
        ///     published ServiceInterface routines and returns null otherwise.
        /// </summary>
        public ulong Sender()
        {
            var caller = Caller.Value;
            if (caller == null)
                throw new InvalidOperationException();
            return caller.ServiceGuid;
        }

        /// <summary>
        ///     Broadcasts a message to all connected remote services.
        /// </summary>
        public void Broadcast(Message message)
        {
            if (message == null)
                throw new ArgumentNullException();
            /// ToDo: Which protocol version to use?
            var stream = new Shift.Network.MessageStream(1);
            stream.Write(message.MessageUid);
            stream.Write(message, null, new Group());
            lock (Sockets)
            {
                foreach (var socket in Sockets)
                    socket.Post(stream);
            }
        }

        /// <summary>
        ///     Returns a serialization interface which internally forwards all calls to
        ///     the remote host which called the last service method.
        /// </summary>
        /// <remarks>
        ///     This is only valid from within a service method to reply to the
        ///     caller. Naturally this doesn't make sense from within another thread.
        /// </remarks>
        [Obsolete("Please use the new message/event based interface.")]
        public Serializer<TRemoteInterface> Reply()
        {
            var caller = Caller.Value;
            if (caller == null)
                throw new InvalidOperationException();
            return new Serializer<TRemoteInterface>(caller.Socket, caller.ProtocolVersion);
        }

        /// <summary>
        ///     Send a message to the remote service, which called the last service event.
        /// </summary>
        /// <remarks>
        ///     This is only valid from within a service event to reply to the
        ///     caller. Naturally this doesn't make sense from within another thread.
        /// </remarks>
        protected void Reply(Message message)
        {
            if (message == null)
                throw new ArgumentNullException();
            var caller = Caller.Value;
            if (caller == null)
                throw new InvalidOperationException();
            /// ToDo: Which protocol version to use?
            var stream = new Shift.Network.MessageStream(1);
            stream.Write(message.MessageUid);
            stream.Write(message, null, new Group());
            caller.Socket.Post(stream);
        }
    }
}
