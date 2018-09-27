using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace DotService
{
    public class Serializer
    {
        public Serializer(uint capacity = 32)
        {
            _buffer = new byte[capacity];
        }

        public Serializer(byte[] buffer)
        {
            _buffer = buffer;
        }

        public unsafe bool Read(ref char value)
        {
            return ReadUsingMarshal(ref value);
        }

        public unsafe void Write(char value)
        {
            WriteUsingMarshal(value);
        }

        public unsafe bool Read(ref sbyte value)
        {
            return ReadUsingMarshal(ref value);
        }

        public unsafe void Write(sbyte value)
        {
            WriteUsingMarshal(value);
        }

        public unsafe bool Read(ref byte value)
        {
            return ReadUsingMarshal(ref value);
        }

        public unsafe void Write(byte value)
        {
            WriteUsingMarshal(value);
        }

        public unsafe bool Read(ref short value)
        {
            return ReadUsingMarshal(ref value);
        }

        public unsafe void Write(short value)
        {
            WriteUsingMarshal(value);
        }

        public unsafe bool Read(ref ushort value)
        {
            return ReadUsingMarshal(ref value);
        }

        public unsafe void Write(ushort value)
        {
            WriteUsingMarshal(value);
        }

        public unsafe bool Read(ref int value)
        {
            return ReadUsingMarshal(ref value);
        }

        public unsafe void Write(int value)
        {
            WriteUsingMarshal(value);
        }

        public unsafe bool Read(ref uint value)
        {
            return ReadUsingMarshal(ref value);
        }

        public unsafe void Write(uint value)
        {
            WriteUsingMarshal(value);
        }

        public unsafe bool Read(ref long value)
        {
            return ReadUsingMarshal(ref value);
        }

        public unsafe void Write(long value)
        {
            WriteUsingMarshal(value);
        }

        public unsafe bool Read(ref ulong value)
        {
            return ReadUsingMarshal(ref value);
        }

        public unsafe void Write(ulong value)
        {
            WriteUsingMarshal(value);
        }

        public unsafe bool Read(ref float value)
        {
            return ReadUsingMarshal(ref value);
        }

        public unsafe void Write(float value)
        {
            WriteUsingMarshal(value);
        }

        public unsafe bool Read(ref double value)
        {
            return ReadUsingMarshal(ref value);
        }

        public unsafe void Write(double value)
        {
            WriteUsingMarshal(value);
        }

        public unsafe bool Read(ref string value)
        {
            uint utf8Length = 0;
            if (!Read(ref utf8Length))
                return false;
            if (utf8Length == 0)
                return true;
            var utf8String = new byte[utf8Length];
            fixed (byte* fixedString = &utf8String[0])
            {
                if (!ReadArray(fixedString, utf8Length))
                    return false;
                value = System.Text.Encoding.UTF8.GetString(utf8String);
                return true;
            }
        }

        public unsafe void Write(string value)
        {
            var utf8String = System.Text.Encoding.UTF8.GetBytes(value);
            var utf8Length = (uint)utf8String.Length;
            Write(utf8Length);
            if (utf8Length == 0)
                return;
            fixed (byte* fixedString = &utf8String[0])
                WriteArray(fixedString, utf8Length);
        }

        public byte[] Data
        {
            get { return _buffer; }
        }

        private unsafe bool ReadUsingMarshal<T>(ref T value) where T : struct
        {
            var size = (uint)Marshal.SizeOf(value);
            if (_buffer.Length - _offset < size)
                return false;
            fixed (byte* source = &_buffer[_offset])
            {
                Marshal.PtrToStructure(new IntPtr(source), value);
            }
            _offset += size;
            return true;
        }

        private unsafe void WriteUsingMarshal<T>(T value) where T : struct
        {
            var size = (uint)Marshal.SizeOf(value);
            if (_buffer.Length - _offset < size)
            {
                Array.Resize(ref _buffer, (int)Math.Max(_buffer.Length * 2,
                    _buffer.Length - _offset + size));
            }
            fixed (byte* target = &_buffer[_offset])
            {
                Marshal.StructureToPtr(value, new IntPtr(target), false);
            }
            _offset += size;
        }

        private unsafe bool ReadArray(byte* destination, uint size)
        {
            if (_buffer.Length - _offset < size)
                return false;
            fixed (byte* fixedSource = &_buffer[_offset])
            {
                byte* source = fixedSource;
                while (size > 0)
                    *(destination++) = *(source++);
            }
            _offset += size;
            return true;
        }

        private unsafe void WriteArray(byte* source, uint size)
        {
            if (_buffer.Length - _offset < size)
            {
                Array.Resize(ref _buffer, (int)Math.Max(_buffer.Length * 2,
                    _buffer.Length - _offset + size));
            }
            fixed (byte* fixedTarget = &_buffer[_offset])
            {
                byte* target = fixedTarget;
                while (size > 0)
                    *(target++) = *(source++);
            }
            _offset += size;
        }

        private byte[] _buffer;
        private uint _offset = 0;
    }

    public static class SerializerExtensions
    {
        /// ToDo: Example, generate one extension method for each required type.
        public static void Write(this Serializer serializer, int[] array)
        {
            foreach (var value in array)
                serializer.Write(value);
        }
    }

    public class SerializerTest
    {
        void Bar()
        {
            int i = 1;
            uint ui = 2;
            int[] ia = new int[5] {1, 2, 3, 4, 5};

            Serializer s = new Serializer();
            s.Write(i);
            s.Write(ui);
            s.Write(ia);
        }
    }
}
