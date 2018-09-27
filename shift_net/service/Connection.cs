using Shift.Network;

namespace Shift.Service
{
    internal class Connection
    {
        public Connection(ServiceBase service, TcpClientSocket socket)
        {
            Service = service;
            Socket = socket;
            if (socket != null)
                ChannelId = socket.ChannelId;
            else
                ChannelId = 0;
        }

        public ServiceBase Service { get; set; }
        public TcpClientSocket Socket { get; set; }
        public uint ChannelId { get; set; }

        public uint ProtocolVersion { get; set; }
        public ulong HostGuid { get; set; }
        public ulong ServiceGuid { get; set; }
        public uint ServiceUid { get; set; }
        public ulong InterfaceUids { get; set; }
        public ushort ServiceTag { get; set; }
    }
}
