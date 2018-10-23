using System;

namespace Shift.Network
{
    /// <summary>
    /// </summary>
    public enum SerialProtocolCodes : byte
    {
        Escape = 0x1b,
        RoundBracketOpen = (byte)'(',
        RoundBracketClose = (byte)')',
        SquaredBracketOpen = (byte)'[',
        SquaredBracketClose = (byte)']',
        CurlyBracketOpen = (byte)'{',
        CurlyBracketClose = (byte)'}',
        AngleBracketOpen = (byte)'<',
        AngleBracketClose = (byte)'>'
    };

    /// <summary>
    ///     Base class for all messages which are to be sent or received through
    ///     the network library.
    /// </summary>
    public abstract class Message : EventArgs
    {
        public uint MessageUid { get; protected set; }
    }
}
