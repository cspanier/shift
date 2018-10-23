using Shift.Network;

namespace Shift.Service
{
    public class SerializerBase
    {
        internal SerializerBase(Socket socket, uint protocolVersion)
        {
            Socket = socket;
            ProtocolVersion = protocolVersion;
        }

        public Socket Socket { get; private set; }
        public uint ProtocolVersion { get; private set; }
    }

    public class Serializer<TServiceInterface> : SerializerBase
    {
        internal Serializer(Socket socket, uint protocolVersion)
            : base(socket, protocolVersion)
        {
        }
    }

    public interface IDeserializer
    {
        void Receive(MessageStream stream, object implementation);
    }
}
