using Shift.Protogen;
using System;
using System.Collections;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;

namespace Shift.Network
{
    public enum PtrType
    {
        ValuePtr,
        RawPtr,
        UniquePtr,
        SharedPtr,
        WeakPtr,
        GroupPtr
    }

    public class MessageStream
    {
        public MessageStream(uint protocolVersion, uint capacity = 32, bool writeLength = true)
        {
            Buffer = new byte[capacity];
            ProtocolVersion = protocolVersion;
            _writeLength = writeLength;
        }

        public MessageStream(byte[] buffer)
        {
            Buffer = buffer;
            Length = buffer.Length;
            ProtocolVersion = 0;
        }

        /// <summary>
        ///     The generic version of Read(Type, object, Group, PtrType).
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="value"></param>
        public void Read<T>(out T value, Type protoType = null, Group cache = null)
        {
            object obj;
            Read(typeof(T), protoType != null ? protoType : typeof(T), out obj, cache);
            value = (T)obj;
        }

        /// <summary>
        ///     The generic version of Write(Type, object, Group, PtrType).
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="value"></param>
        public void Write<T>(T value, Type protoType = null, Group cache = null)
        {
            Write(value != null ? value.GetType() : typeof(T), protoType != null ? protoType : typeof(T), (object)value, cache);
        }

        /// <summary>
        ///     A wrapper for Read(Type, out object, Group), which takes care of any
        ///     special pointer deserialization.
        /// </summary>
        /// <param name="type"></param>
        /// <param name="obj"></param>
        /// <param name="cache"></param>
        /// <param name="ptrType"></param>
        private void Read(Type type, Type protoType, out object obj, Group cache)
        {
            PtrType ptrType = PtrType.ValuePtr;
            if (protoType.GetTypeInfo().IsGenericType)
            {
                var genericProtoType = protoType.GetGenericTypeDefinition();
                if (genericProtoType == typeof(RawPtr<>))
                {
                    protoType = protoType.GenericTypeArguments.First();
                    ptrType = PtrType.RawPtr;
                }
                else if (genericProtoType == typeof(UniquePtr<>))
                {
                    protoType = protoType.GenericTypeArguments.First();
                    ptrType = PtrType.UniquePtr;
                }
                else if (genericProtoType == typeof(SharedPtr<>))
                {
                    protoType = protoType.GenericTypeArguments.First();
                    ptrType = PtrType.SharedPtr;
                }
                else if (genericProtoType == typeof(WeakPtr<>))
                {
                    protoType = protoType.GenericTypeArguments.First();
                    ptrType = PtrType.WeakPtr;
                }
                else if (genericProtoType == typeof(GroupPtr<>))
                {
                    protoType = protoType.GenericTypeArguments.First();
                    ptrType = PtrType.GroupPtr;
                }
            }

            switch (ptrType)
            {
                case PtrType.ValuePtr:
                    ReadSwitch(type, protoType, out obj, cache, 0);
                    break;

                case PtrType.RawPtr:
                case PtrType.UniquePtr:
                    bool exists;
                    Read(out exists, null, cache);
                    if (exists)
                        ReadSwitch(type, protoType, out obj, cache, 0);
                    else
                        obj = null;
                    break;

                case PtrType.SharedPtr:
                case PtrType.WeakPtr:
                case PtrType.GroupPtr:
                    if (cache == null)
                        throw new ArgumentNullException();

                    ushort key;
                    Read(out key, null, cache);
                    obj = key != 0 ? cache.Find(key) : null;
                    if (key != 0 && obj == null)
                        ReadSwitch(type, protoType, out obj, cache, key);
                    break;

                default:
                    obj = null; // Needed to silence error about obj not being initialized below, which is nonesense.
                    throw new NotImplementedException();
            }
        }

        /// <summary>
        ///     A wrapper for Write(Type, object, Group), which takes care of any
        ///     special pointer serialization.
        /// </summary>
        /// <param name="type"></param>
        /// <param name="obj"></param>
        /// <param name="cache"></param>
        /// <param name="ptrType"></param>
        private void Write(Type type, Type protoType, object obj, Group cache)
        {
            PtrType ptrType = PtrType.ValuePtr;
            if (protoType.GetTypeInfo().IsGenericType)
            {
                var genericProtoType = protoType.GetGenericTypeDefinition();
                if (genericProtoType == typeof(RawPtr<>))
                {
                    protoType = protoType.GenericTypeArguments.First();
                    ptrType = PtrType.RawPtr;
                }
                else if (genericProtoType == typeof(UniquePtr<>))
                {
                    protoType = protoType.GenericTypeArguments.First();
                    ptrType = PtrType.UniquePtr;
                }
                else if (genericProtoType == typeof(SharedPtr<>))
                {
                    protoType = protoType.GenericTypeArguments.First();
                    ptrType = PtrType.SharedPtr;
                }
                else if (genericProtoType == typeof(WeakPtr<>))
                {
                    protoType = protoType.GenericTypeArguments.First();
                    ptrType = PtrType.WeakPtr;
                }
                else if (genericProtoType == typeof(GroupPtr<>))
                {
                    protoType = protoType.GenericTypeArguments.First();
                    ptrType = PtrType.GroupPtr;
                }
            }

            switch (ptrType)
            {
                case PtrType.ValuePtr:
                    WriteSwitch(type, protoType, obj, cache);
                    break;

                case PtrType.RawPtr:
                case PtrType.UniquePtr:
                    if (obj != null)
                    {
                        Write(true, null, cache);
                        WriteSwitch(type, protoType, obj, cache);
                    }
                    else
                        Write(false, null, cache);
                    break;

                case PtrType.SharedPtr:
                case PtrType.WeakPtr:
                case PtrType.GroupPtr:
                    if (obj != null)
                    {
                        ushort key = cache.Find(obj);
                        if (key == 0)
                        {
                            key = cache.Add(obj);
                            Write(key, null, cache);
                            WriteSwitch(type, protoType, obj, cache);
                        }
                        else
                            Write(key, null, cache);
                    }
                    else
                        Write((ushort)0, null, cache);
                    break;

                default:
                    throw new NotImplementedException();
            }
        }

        /// <summary>
        ///     A switching method into each of the specific specializations.
        /// </summary>
        /// <param name="type"></param>
        /// <param name="obj"></param>
        /// <param name="cache"></param>
        private void ReadSwitch(Type type, Type protoType, out object obj, Group cache, ushort key)
        {
            TypeInfo typeInfo = type.GetTypeInfo();

            if (type == typeof(bool))
                ReadBoolean(out obj, cache, key);
            else if (type == typeof(char))
                ReadChar(out obj, cache, key);
            else if (type.IsFundamental())
                ReadFundamental(type, out obj, cache, key);
            else if (typeInfo.IsEnum)
                ReadEnum(type, out obj, cache, key);
            else if (type == typeof(String))
                obj = ReadString(cache, key);
            else if (type == typeof(DateTime))
                obj = ReadDateTime(cache, key);
            else if (typeof(Message).IsAssignableFrom(type))
                obj = ReadMessage(type, protoType, cache, key);
            else if (typeInfo.IsGenericType && type.GetGenericTypeDefinition() == typeof(Nullable<>))
                obj = ReadNullable(type, protoType, cache);
            else if (type.IsArray)
                obj = ReadArray(type, protoType, cache, key);
            else if (typeInfo.IsGenericType &&
                (type.GetGenericTypeDefinition() == typeof(List<>)
                    || type.GetGenericTypeDefinition() == typeof(ObservableCollection<>)))
            {
                obj = ReadList(type, protoType, cache, key);
            }
            else if (typeInfo.IsGenericType &&
                type.GetGenericTypeDefinition() == typeof(Dictionary<,>))
            {
                obj = ReadDictionary(type, protoType, cache, key);
            }
            else if (typeInfo.IsGenericType &&
                type.GetGenericTypeDefinition() == typeof(HashSet<>))
            {
                obj = ReadSet(type, protoType, cache, key);
            }
            else if (typeof(IVariant).IsAssignableFrom(type))
                obj = ReadVariant(type, protoType, cache, key);
            else if (type == typeof(System.Windows.Point))
                ReadPoint2D(type, out obj, cache, key);
            else if (type == typeof(Thales.Sagittarius.Data.Media3D.Point3D))
                ReadPoint3D(type, out obj, cache, key);
            else if (type == typeof(Thales.Sagittarius.Data.Media3D.Matrix3D))
                ReadMatrix3D(type, out obj, cache, key);
            else
            {
                var aliasAttribute = typeInfo.GetCustomAttribute<AliasAttribute>(false);
                if (aliasAttribute == null)
                {
                    throw new NotImplementedException(String.Format(
                        "Missing deserialization support for type {0}!", type.FullName));
                }
                obj = ReadAlias(type, aliasAttribute.AliasType, protoType, cache, key);
            }
        }

        /// <summary>
        ///     A switching method into each of the specific specializations.
        /// </summary>
        /// <param name="type"></param>
        /// <param name="obj"></param>
        /// <param name="cache"></param>
        private void WriteSwitch(Type type, Type protoType, object obj, Group cache)
        {
            TypeInfo typeInfo = type.GetTypeInfo();

            if (type == typeof(bool))
            {
                if (obj == null)
                    throw new ArgumentNullException();
                WriteBoolean((Boolean)obj);
            }
            else if (type == typeof(char))
            {
                if (obj == null)
                    throw new ArgumentNullException();
                WriteChar((Char)obj);
            }
            else if (type.IsFundamental())
            {
                if (obj == null)
                    throw new ArgumentNullException();
                WriteFundamental(type, obj);
            }
            else if (typeInfo.IsEnum)
            {
                if (obj == null)
                    throw new ArgumentNullException();
                WriteEnum(type, obj);
            }
            else if (type == typeof(String))
                WriteString((String)obj);
            else if (type == typeof(DateTime))
            {
                if (obj == null)
                    throw new ArgumentNullException();
                WriteDateTime((DateTime)obj);
            }
            else if (typeof(Message).IsAssignableFrom(type))
            {
                if (obj == null)
                    throw new ArgumentNullException();
                WriteMessage(type, protoType, (Message)obj, cache);
            }
            else if (typeInfo.IsGenericType && type.GetGenericTypeDefinition() == typeof(Nullable<>))
                WriteNullable(type, protoType, obj, cache);
            else if (type.IsArray)
                WriteArray(type, protoType, (Array)obj, cache);
            else if (typeInfo.IsGenericType &&
                (type.GetGenericTypeDefinition() == typeof(List<>)
                || type.GetGenericTypeDefinition() == typeof(ObservableCollection<>)))
            {
                WriteList(type, protoType, (IList)obj, cache);
            }
            else if (typeInfo.IsGenericType &&
                type.GetGenericTypeDefinition() == typeof(Dictionary<,>))
            {
                WriteDictionary(type, protoType, (IDictionary)obj, cache);
            }
            else if (typeInfo.IsGenericType &&
                type.GetGenericTypeDefinition() == typeof(HashSet<>))
            {
                WriteSet(type, protoType, (IEnumerable)obj, cache);
            }
            else if (typeof(IVariant).IsAssignableFrom(type))
            {
                WriteVariant(type, protoType, (IVariant)obj, cache);
            }
            else if (type == typeof(System.Windows.Point))
                WritePoint2D(type, (System.Windows.Point)obj);
            else if (type == typeof(Thales.Sagittarius.Data.Media3D.Point3D))
                WritePoint3D(type, (Thales.Sagittarius.Data.Media3D.Point3D)obj);
            else if (type == typeof(Thales.Sagittarius.Data.Media3D.Matrix3D))
                WriteMatrix3D(type, (Thales.Sagittarius.Data.Media3D.Matrix3D)obj);
            else
            {
                var aliasAttribute = typeInfo.GetCustomAttribute<AliasAttribute>(false);
                if (aliasAttribute == null)
                {
                    throw new NotImplementedException(String.Format(
                        "Missing serialization support for type {0}!", type.FullName));
                }
                WriteAlias(type, aliasAttribute.AliasType, protoType, obj, cache);
            }
            if (Offset > Length)
                Length = Offset;
        }

        /// <summary>
        ///     Deserialization method for type bool.
        /// </summary>
        /// <remarks>
        ///     This specialization is needed because sizeof(bool) equals 4.
        /// </remarks>
        private unsafe void ReadBoolean(out object value, Group cache, ushort key)
        {
            if (Length < Offset + 1)
                throw new EndOfStreamException();
            value = Buffer[Offset++] != 0;
            if (key != 0)
                cache.Add(key, value);
        }

        /// <summary>
        ///     Serialization method for type bool.
        /// </summary>
        /// <remarks>
        ///     This specialization is needed because sizeof(bool) equals 4.
        /// </remarks>
        private unsafe void WriteBoolean(Boolean value)
        {
            if (Buffer.Length < Offset + 1)
                Resize((Buffer.Length + 1) * 2);
            Buffer[Offset++] = value ? (byte)1 : (byte)0;
        }

        /// <summary>
        ///     Deserialization method for type char.
        /// </summary>
        /// <remarks>
        ///     This specialization is needed because Marshal.SizeOf(typeof(char))
        ///     returns 1 instead of 2.
        /// </remarks>
        private unsafe void ReadChar(out object value, Group cache, ushort key)
        {
            object word;
            ReadFundamental(typeof(short), out word, cache, 0);
            value = Convert.ToChar(word);
            if (key != 0)
                cache.Add(key, value);
        }

        /// <summary>
        ///     Serialization method for type char.
        /// </summary>
        /// <remarks>
        ///     This specialization is needed because Marshal.SizeOf(typeof(char))
        ///     returns 1 instead of 2.
        /// </remarks>
        private unsafe void WriteChar(Char value)
        {
            WriteFundamental(typeof(short), Convert.ToInt16(value));
        }

        /// <summary>
        ///     Deserialization method for any fundamental value type T.
        /// </summary>
        private unsafe void ReadFundamental(Type type, out object value, Group cache, ushort key)
        {
            var size = Marshal.SizeOf(type);
            if (Length < Offset + size)
                throw new EndOfStreamException();
            fixed (byte* pointer = &Buffer[Offset])
            {
                value = Marshal.PtrToStructure(new IntPtr(pointer), type);
            }
            if (key != 0)
                cache.Add(key, value);
            Offset += size;
        }

        /// <summary>
        ///     Serialization method for any fundamental value type T.
        /// </summary>
        private unsafe void WriteFundamental(Type type, object value)
        {
            var size = Marshal.SizeOf(type);
            if (Buffer.Length < Offset + size)
                Resize((Buffer.Length + size) * 2);
            fixed (byte* pointer = &Buffer[Offset])
            {
                Marshal.StructureToPtr(value, new IntPtr(pointer), false);
            }
            Offset += size;
        }

        /// <summary>
        ///     Deserialization method for any enum value type.
        /// </summary>
        private unsafe void ReadEnum(Type type, out object value, Group cache, ushort key)
        {
            var underlyingType = Enum.GetUnderlyingType(type);
            var size = Marshal.SizeOf(underlyingType);
            if (Length < Offset + size)
                throw new EndOfStreamException();
            fixed (byte* pointer = &Buffer[Offset])
            {
                var underlyingValue = Marshal.PtrToStructure(new IntPtr(pointer), underlyingType);
                value = Enum.ToObject(type, underlyingValue);
            }
            if (key != 0)
                cache.Add(key, value);
            Offset += size;
        }

        /// <summary>
        ///     Serialization method for any enum value type.
        /// </summary>
        private unsafe void WriteEnum(Type type, object value)
        {
            var underlyingType = Enum.GetUnderlyingType(type);
            var size = Marshal.SizeOf(underlyingType);
            if (Buffer.Length < Offset + size)
                Resize((Buffer.Length + size) * 2);
            fixed (byte* pointer = &Buffer[Offset])
            {
                Marshal.StructureToPtr(Convert.ChangeType(value, underlyingType), new IntPtr(pointer), false);
            }
            Offset += size;
        }

        private object ReadNullable(Type type, Type protoType, Group cache)
        {
            var elementType = type.GenericTypeArguments.First();
            object element;
            Read(elementType, protoType, out element, cache);

            return type.GetConstructor(new Type[] { elementType }).
                Invoke(new object[] { element });
        }

        private void WriteNullable(Type type, Type protoType, object value, Group cache)
        {
            var elementType = type.GenericTypeArguments.First();
            Write(elementType, protoType, value, cache);
        }

        /// <summary>
        ///     A specialization for reading dynamically sized length values.
        /// </summary>
        /// <param name="value"></param>
        public void ReadLength(out uint value)
        {
            value = 0;
            for (var i = 0; i < 5; ++i)
            {
                if (Offset >= Length)
                    throw new EndOfStreamException();
                byte digit = Buffer[Offset++];
                value |= (uint)(digit & 0x7f) << (i * 7);
                if ((digit & 0x80) == 0)
                    break;
            }
        }

        /// <summary>
        ///     A specialization for writing dynamically sized length values.
        /// </summary>
        /// <param name="value"></param>
        public void WriteLength(uint value)
        {
            if (!_writeLength)
                return;
            do
            {
                byte digit = (byte)(value & 0x7F);
                if (value > 0x7F)
                    digit |= 0x80;
                if (Buffer.Length <= Offset)
                    Resize((Buffer.Length + 4) * 2);
                Buffer[Offset++] = digit;
                value >>= 7;
            } while (value > 0);
        }

        /// <summary>
        ///     A specialization to deserialize strings.
        /// </summary>
        /// <returns></returns>
        private string ReadString(Group cache, ushort key)
        {
            uint length;
            ReadLength(out length);
            if (length == 0)
                return null; // ToDo: Empty or null?

            if (Length - Offset < length)
                throw new EndOfStreamException();

            var utf8String = new byte[length];
            Array.Copy(Buffer, (int)Offset, utf8String, 0, (int)length);
            Offset += length;
            var result = System.Text.Encoding.UTF8.GetString(utf8String);
            if (key != 0)
                cache.Add(key, result);
            return result;
        }

        /// <summary>
        ///     A specialization to serialize strings.
        /// </summary>
        /// <param name="str"></param>
        private void WriteString(string str)
        {
            if (String.IsNullOrEmpty(str))
            {
                WriteLength(0);
                return;
            }
            var utf8String = System.Text.Encoding.UTF8.GetBytes(str);
            var length = (uint)utf8String.Length;
            WriteLength(length);

            if (length > 0)
            {
                if (Buffer.Length < length + Offset)
                    Resize((2 * Buffer.Length + length - Offset) * 2);
                Array.Copy(utf8String, 0, Buffer, (int)Offset, (int)length);
                Offset += length;
            }
        }

        /// <summary>
        ///     A specialization to deserialize types derived from type Message.
        /// </summary>
        /// <param name="type"></param>
        /// <returns></returns>
        private Message ReadMessage(Type type, Type protoType, Group cache, ushort key)
        {
            var message = (Message)Activator.CreateInstance(type);
            if (key != 0)
                cache.Add(key, message);

            SortedList<int, Tuple<PropertyInfo, Type>> properties;

            if (!s_typePropertyDic.TryGetValue(type, out properties))
            {
                properties = new SortedList<int, Tuple<PropertyInfo, Type>>();
                foreach (var property in type.GetProperties())
                {
                    var orderAttribute = property.GetCustomAttribute<OrderAttribute>(true);
                    if (orderAttribute == null)
                        continue;
                    var protoTypeFromAttribute = property.
                        GetCustomAttribute<ProtoTypeAttribute>(true).ProtoType;
                    properties.Add(orderAttribute.Order,
                        new Tuple<PropertyInfo, Type>(property, protoTypeFromAttribute));
                }
                s_typePropertyDic.TryAdd(type, properties);
            }

            foreach (var propertyTuple in properties.Values)
            {
                object value;
                Read(propertyTuple.Item1.PropertyType,
                    propertyTuple.Item2,
                    out value, cache);
                propertyTuple.Item1.SetValue(message, value);
            }
            return message;
        }

        /// <summary>
        ///     A specialization to serialize types derived from type Message.
        /// </summary>
        /// <param name="message"></param>
        private void WriteMessage(Type type, Type protoType, Message message, Group cache)
        {
            SortedList<int, Tuple<PropertyInfo, Type>> properties;

            if (!s_typePropertyDic.TryGetValue(type, out properties))
            {
                properties = new SortedList<int, Tuple<PropertyInfo, Type>>();
                foreach (var property in type.GetProperties())
                {
                    var orderAttribute = property.GetCustomAttribute<OrderAttribute>(true);
                    if (orderAttribute == null)
                        continue;
                    var protoTypeFromAttribute = property.
                        GetCustomAttribute<ProtoTypeAttribute>(true).ProtoType;
                    properties.Add(orderAttribute.Order,
                        new Tuple<PropertyInfo, Type>(property, protoTypeFromAttribute));
                }
                s_typePropertyDic.TryAdd(type, properties);
            }

            foreach (var propertyTuple in properties.Values)
            {
                Write(propertyTuple.Item1.PropertyType,
                    propertyTuple.Item2,
                    propertyTuple.Item1.GetValue(message), cache);
            }
        }

        /// <summary>
        ///     A specialization for alias type deserialization.
        /// </summary>
        /// <param name="type"></param>
        /// <returns></returns>
        private object ReadAlias(Type type, Type aliasType, Type protoType, Group cache, ushort key)
        {
            object value;
            Read(aliasType, protoType, out value, cache);
            return type.
                GetConstructor(new Type[] { aliasType }).
                Invoke(new object[] { value });
        }

        /// <summary>
        ///     A specialization for alias type serialization.
        /// </summary>
        /// <param name="type"></param>
        /// <param name="aliasType"></param>
        /// <param name="value"></param>
        private void WriteAlias(Type type, Type aliasType, Type protoType, object value, Group cache)
        {
            Write(aliasType, protoType, value, cache);
        }

        /// <summary>
        ///     A specialization for array deserialization.
        /// </summary>
        /// <param name="type"></param>
        /// <returns></returns>
        private Array ReadArray(Type type, Type protoType, Group cache, ushort key)
        {
            uint length;
            ReadLength(out length);
            var elementType = type.GetElementType();
            var array = Array.CreateInstance(elementType, (int)length);
            if (key != 0)
                cache.Add(key, array);
            if (length == 0)
                return array; // ToDo: Empty or null?
            if (elementType.IsFundamental())
            {
                var lengthInBytes = length * (elementType != typeof(bool) ? Marshal.SizeOf(elementType) : 1);
                if (Length < Offset + lengthInBytes)
                    throw new EndOfStreamException();
                if (Offset > Int32.MaxValue || lengthInBytes > Int32.MaxValue)
                    throw new OverflowException();
                System.Buffer.BlockCopy(Buffer, (int)Offset, array, 0, (int)lengthInBytes);
                Offset += lengthInBytes;
            }
            else
            {
                for (int index = 0; index < length; ++index)
                {
                    object element;
                    Read(elementType, protoType.GetElementType(), out element, cache);
                    array.SetValue(element, index);
                }
            }
            return array;
        }

        /// <summary>
        ///     A specialization for array serialization.
        /// </summary>
        /// <param name="type"></param>
        /// <param name="array"></param>
        private void WriteArray(Type type, Type protoType, Array array, Group cache)
        {
            if (array == null)
            {
                WriteLength(0);
                return;
            }
            var elementType = type.GetElementType();
            WriteLength((uint)array.Length);
            if (elementType.IsFundamental())
            {
                var lengthInBytes = array.Length * (elementType != typeof(bool) ? Marshal.SizeOf(elementType) : 1);

                if (Buffer.Length < Offset + lengthInBytes)
                    Resize((Buffer.Length + lengthInBytes) * 2);
                if (Offset > Int32.MaxValue || lengthInBytes > Int32.MaxValue)
                    throw new OverflowException();
                System.Buffer.BlockCopy(array, 0, Buffer, (int)Offset, (int)lengthInBytes);
                Offset += lengthInBytes;
            }
            else
            {
                foreach (var element in array)
                    Write(elementType, protoType.GetElementType(), element, cache);
            }
        }

        /// <summary>
        ///     A specialization for list deserialization.
        /// </summary>
        /// <param name="type"></param>
        /// <returns></returns>
        private IList ReadList(Type type, Type protoType, Group cache, ushort key)
        {
            uint length;
            ReadLength(out length);

            var elementType = type.GenericTypeArguments.First();
            if (elementType.IsFundamental())
            {
                // C# 
                var lengthInBytes = length * (elementType != typeof(bool) ? Marshal.SizeOf(elementType) : 1);
                if (Length < Offset + lengthInBytes)
                    throw new EndOfStreamException();
                if (Offset > Int32.MaxValue || lengthInBytes > Int32.MaxValue)
                    throw new OverflowException();
                var array = Array.CreateInstance(elementType, (int)length);
                System.Buffer.BlockCopy(Buffer, (int)Offset, array, 0, (int)lengthInBytes);
                Offset += lengthInBytes;

                var list = (IList)type.GetGenericTypeDefinition().
                    MakeGenericType(elementType).
                    GetConstructor(new Type[] { typeof(IEnumerable<>).MakeGenericType(elementType) }).
                    Invoke(new object[] { array });
                if (key != 0)
                    cache.Add(key, list);
                return list;
            }
            else
            {
                var list = (IList)type.GetGenericTypeDefinition().
                    MakeGenericType(elementType).
                    GetConstructor(Type.EmptyTypes).
                    Invoke(null);
                if (key != 0)
                    cache.Add(key, list);
                for (int index = 0; index < length; ++index)
                {
                    object element;
                    Read(elementType, protoType.GenericTypeArguments.First(), out element, cache);
                    list.Add(element);
                }
                return list;
            }
        }

        /// <summary>
        ///     A specialization for list serialization.
        /// </summary>
        /// <param name="type"></param>
        /// <param name="list"></param>
        private void WriteList(Type type, Type protoType, IList list, Group cache)
        {
            var elementType = type.GenericTypeArguments.First();
            if (list == null)
            {
                WriteLength(0);
                return;
            }
            WriteLength((uint)list.Count);
            if (elementType.IsFundamental())
            {
                var array = Array.CreateInstance(elementType, list.Count);
                list.CopyTo(array, 0);

                var lengthInBytes = array.Length * (elementType != typeof(bool) ? Marshal.SizeOf(elementType) : 1);

                if (Buffer.Length < Offset + lengthInBytes)
                    Resize((Buffer.Length + lengthInBytes) * 2);
                if (Offset > Int32.MaxValue || lengthInBytes > Int32.MaxValue)
                    throw new OverflowException();
                System.Buffer.BlockCopy(array, 0, Buffer, (int)Offset, (int)lengthInBytes);
                Offset += lengthInBytes;
            }
            else
            {
                foreach (var element in list)
                    Write(elementType, protoType.GenericTypeArguments.First(), element, cache);
            }
        }

        /// <summary>
        ///     A specialization for dictionary deserialization.
        /// </summary>
        /// <param name="type"></param>
        /// <returns></returns>
        private IDictionary ReadDictionary(Type type, Type protoType, Group cache, ushort key)
        {
            uint length;
            ReadLength(out length);

            var keyType = type.GenericTypeArguments.First();
            var valueType = type.GenericTypeArguments.Last();
            var keyProtoType = protoType.GenericTypeArguments.First();
            var valueProtoType = protoType.GenericTypeArguments.Last();
            var dictionary = (IDictionary)typeof(Dictionary<,>).
                MakeGenericType(new Type[] { keyType, valueType }).
                GetConstructor(Type.EmptyTypes).
                Invoke(null);
            if (length == 0)
                return dictionary; // ToDo: Empty or null?
            if (key != 0)
                cache.Add(key, dictionary);
            for (int index = 0; index < length; ++index)
            {
                object elementKey;
                object elementValue;
                Read(keyType, keyProtoType, out elementKey, cache);
                Read(valueType, valueProtoType, out elementValue, cache);
                dictionary.Add(elementKey, elementValue);
            }
            return dictionary;
        }

        /// <summary>
        ///     A specialization for dictionary serialization.
        /// </summary>
        /// <param name="type"></param>
        /// <param name="dictionary"></param>
        private void WriteDictionary(Type type, Type protoType, IDictionary dictionary, Group cache)
        {
            var keyType = type.GenericTypeArguments.First();
            var valueType = type.GenericTypeArguments.Last();
            var keyProtoType = protoType.GenericTypeArguments.First();
            var valueProtoType = protoType.GenericTypeArguments.Last();
            if (dictionary == null)
            {
                WriteLength(0);
                return;
            }
            WriteLength((uint)dictionary.Count);
            foreach (DictionaryEntry element in dictionary)
            {
                Write(keyType, keyProtoType, element.Key, cache);
                Write(valueType, valueProtoType, element.Value, cache);
            }
        }

        /// <summary>
        ///     A specialization for set deserialization.
        /// </summary>
        /// <param name="type"></param>
        /// <returns></returns>
        private IEnumerable ReadSet(Type type, Type protoType, Group cache, ushort key)
        {
            uint length;
            ReadLength(out length);

            var elementType = type.GenericTypeArguments.First();
            var elementProtoType = protoType.GenericTypeArguments.First();
            var hashSetType = typeof(HashSet<>).MakeGenericType(elementType);
            var set = (IEnumerable)hashSetType.
                GetConstructor(Type.EmptyTypes).
                Invoke(null);
            if (length == 0)
                return set; // ToDo: Empty or null?
            if (key != 0)
                cache.Add(key, set);
            var add = hashSetType.GetMethod("Add");
            for (int index = 0; index < length; ++index)
            {
                object element;
                Read(elementType, elementProtoType, out element, cache);
                add.Invoke(set, new object[] { element });
            }
            return set;
        }

        /// <summary>
        ///     A specialization for set serialization.
        /// </summary>
        /// <param name="type"></param>
        /// <param name="set"></param>
        private void WriteSet(Type type, Type protoType, IEnumerable set, Group cache)
        {
            if (set == null)
            {
                WriteLength(0);
                return;
            }
            var elementType = type.GenericTypeArguments.First();
            var elementProtoType = protoType.GenericTypeArguments.First();
            var hashSetType = typeof(HashSet<>).MakeGenericType(elementType);
            var count = Convert.ToInt32(hashSetType.GetProperty("Count").GetValue(set));
            WriteLength((uint)count);
            foreach (var element in set)
                Write(elementType, elementProtoType, element, cache);
        }

        /// <summary>
        ///     A specialization for Variant deserialization.
        /// </summary>
        /// <param name="type"></param>
        /// <returns></returns>
        private IVariant ReadVariant(Type type, Type protoType, Group cache, ushort key)
        {
            byte which;
            Read(out which, null, cache);
            var variant = (IVariant)type.GetConstructor(Type.EmptyTypes).Invoke(null);
            var elementType = variant.GetNthType(which);
            object element;
            while (protoType.GenericTypeArguments.Count() == 0)
                protoType = protoType.BaseType;
            while (which >= 7)
            {
                protoType = protoType.GenericTypeArguments[7];
                which -= 7;
            }
            Read(elementType, protoType.GenericTypeArguments[which], out element, cache);
            variant.SetValue(element);
            return variant;
        }

        /// <summary>
        ///     A specialization for Variant serialization.
        /// </summary>
        /// <param name="type"></param>
        /// <param name="variant"></param>
        private void WriteVariant(Type type, Type protoType, IVariant variant, Group cache)
        {
            byte which = (byte)variant.Which();
            Write(which, null, cache);
            while (protoType.GenericTypeArguments.Count() == 0)
                protoType = protoType.BaseType;
            while (which >= 7)
            {
                protoType = protoType.GenericTypeArguments[7];
                which -= 7;
            }
            Write(variant.GetValue(), protoType.GenericTypeArguments[which], cache);
        }

        /// <summary>
        ///     A specialization for System.Windows.Point deserialization.
        /// </summary>
        /// <param name="type"></param>
        /// <returns></returns>
        private unsafe void ReadPoint2D(Type type, out object value, Group cache, ushort key)
        {
            Type singleType = typeof(Single);
            var size = 2 * sizeof(Single);
            if (Length < Offset + size)
                throw new EndOfStreamException();
            var point2D = new System.Windows.Point();

            double element;
            readDoubleFromSingle(out element, cache);
            point2D.X = element;
            readDoubleFromSingle(out element, cache);
            point2D.Y = element;

            value = point2D;
            if (key != 0)
                cache.Add(key, value);
        }

        /// <summary>
        ///     A specialization for System.Windows.Point serialization.
        /// </summary>
        /// <param name="type"></param>
        /// <param name="array"></param>
        private unsafe void WritePoint2D(Type type, System.Windows.Point point2D)
        {
            if (point2D == null)
            {
                return;
            }

            WriteFundamental(typeof(Single), System.Convert.ToSingle(point2D.X));
            WriteFundamental(typeof(Single), System.Convert.ToSingle(point2D.Y));
        }

        /// <summary>
        ///     A specialization for Thales.Sagittarius.Data.Media3D.Point3D deserialization.
        /// </summary>
        /// <param name="type"></param>
        /// <returns></returns>
        private unsafe void ReadPoint3D(Type type, out object value, Group cache, ushort key)
        {
            Type singleType = typeof(Single);
            var size = 3 * sizeof(Single);
            if (Length < Offset + size)
                throw new EndOfStreamException();
            var point3D = new Thales.Sagittarius.Data.Media3D.Point3D();

            double element;
            readDoubleFromSingle(out element, cache);
            point3D.X = element;
            readDoubleFromSingle(out element, cache);
            point3D.Y = element;
            readDoubleFromSingle(out element, cache);
            point3D.Z = element;

            value = point3D;
            if (key != 0)
                cache.Add(key, value);
        }

        /// <summary>
        ///     A specialization for Thales.Sagittarius.Data.Media3D.Point3D serialization.
        /// </summary>
        /// <param name="type"></param>
        /// <param name="array"></param>
        private unsafe void WritePoint3D(Type type, Thales.Sagittarius.Data.Media3D.Point3D point3D)
        {
            if (point3D == null)
            {
                return;
            }

            WriteFundamental(typeof(Single), System.Convert.ToSingle(point3D.X));
            WriteFundamental(typeof(Single), System.Convert.ToSingle(point3D.Y));
            WriteFundamental(typeof(Single), System.Convert.ToSingle(point3D.Z));
        }

        /// <summary>
        ///     A specialization for Thales.Sagittarius.Data.Media3D.Marix3D deserialization.
        /// </summary>
        /// <param name="type"></param>
        /// <returns></returns>
        private unsafe void ReadMatrix3D(Type type, out object value, Group cache, ushort key)
        {
            Type singleType = typeof(Single);
            var size = 4 * 4 * sizeof(Single);
            if (Length < Offset + size)
                throw new EndOfStreamException();
            var matrix3D = new Thales.Sagittarius.Data.Media3D.Matrix3D();

            double element;

            readDoubleFromSingle(out element, cache);
            matrix3D.M11 = element;

            readDoubleFromSingle(out element, cache);
            matrix3D.M12 = element;

            readDoubleFromSingle(out element, cache);
            matrix3D.M13 = element;

            readDoubleFromSingle(out element, cache);
            matrix3D.M14 = element;

            readDoubleFromSingle(out element, cache);
            matrix3D.M21 = element;

            readDoubleFromSingle(out element, cache);
            matrix3D.M22 = element;

            readDoubleFromSingle(out element, cache);
            matrix3D.M23 = element;

            readDoubleFromSingle(out element, cache);
            matrix3D.M24 = element;

            readDoubleFromSingle(out element, cache);
            matrix3D.M31 = element;

            readDoubleFromSingle(out element, cache);
            matrix3D.M32 = element;

            readDoubleFromSingle(out element, cache);
            matrix3D.M33 = element;

            readDoubleFromSingle(out element, cache);
            matrix3D.M34 = element;

            readDoubleFromSingle(out element, cache);
            matrix3D.OffsetX = element;

            readDoubleFromSingle(out element, cache);
            matrix3D.OffsetY = element;

            readDoubleFromSingle(out element, cache);
            matrix3D.OffsetZ = element;

            readDoubleFromSingle(out element, cache);
            matrix3D.M44 = element;

            value = matrix3D;
            if (key != 0)
                cache.Add(key, value);
        }

        private void readDoubleFromSingle(out double element, Group cache)
        {
            object value;
            ReadFundamental(typeof(Single), out value, cache, 0);
            element = System.Convert.ToDouble(value);
        }

        /// <summary>
        ///     A specialization for Thales.Sagittarius.Data.Media3D.Matrix3D serialization.
        /// </summary>
        /// <param name="type"></param>
        /// <param name="array"></param>
        private unsafe void WriteMatrix3D(Type type, Thales.Sagittarius.Data.Media3D.Matrix3D matrix3D)
        {
            if (matrix3D == null)
            {
                return;
            }

            WriteFundamental(typeof(Single), System.Convert.ToSingle(matrix3D.M11));
            WriteFundamental(typeof(Single), System.Convert.ToSingle(matrix3D.M12));
            WriteFundamental(typeof(Single), System.Convert.ToSingle(matrix3D.M13));
            WriteFundamental(typeof(Single), System.Convert.ToSingle(matrix3D.M14));
            WriteFundamental(typeof(Single), System.Convert.ToSingle(matrix3D.M21));
            WriteFundamental(typeof(Single), System.Convert.ToSingle(matrix3D.M22));
            WriteFundamental(typeof(Single), System.Convert.ToSingle(matrix3D.M23));
            WriteFundamental(typeof(Single), System.Convert.ToSingle(matrix3D.M24));
            WriteFundamental(typeof(Single), System.Convert.ToSingle(matrix3D.M31));
            WriteFundamental(typeof(Single), System.Convert.ToSingle(matrix3D.M32));
            WriteFundamental(typeof(Single), System.Convert.ToSingle(matrix3D.M33));
            WriteFundamental(typeof(Single), System.Convert.ToSingle(matrix3D.M34));
            WriteFundamental(typeof(Single), System.Convert.ToSingle(matrix3D.OffsetX));
            WriteFundamental(typeof(Single), System.Convert.ToSingle(matrix3D.OffsetY));
            WriteFundamental(typeof(Single), System.Convert.ToSingle(matrix3D.OffsetZ));
            WriteFundamental(typeof(Single), System.Convert.ToSingle(matrix3D.M44));
        }

        private static DateTime s_epoch = new DateTime(1970, 1, 1, 0, 0, 0, 0);

        /// <summary>
        ///     A specialization for type DateTime.
        /// </summary>
        /// <returns></returns>
        private DateTime ReadDateTime(Group cache, ushort key)
        {
            ulong count;
            Read(out count, null);
            var result = s_epoch.AddMilliseconds(count / 1000000);
            if (key != 0)
                cache.Add(key, result);
            return result;
        }

        /// <summary>
        ///     A specialization for type DateTime, which stores the number of
        ///     nanoseconds since 1970-01-01 00:00.
        /// </summary>
        /// <param name="dateTime"></param>
        private void WriteDateTime(DateTime dateTime)
        {
            if (dateTime != null)
                Write((ulong)(dateTime - s_epoch).TotalMilliseconds * 1000000, null);
            else
                Write((ulong)0, null);
        }

        ///// <summary>
        /////     Fallback serialization of unknown types.
        ///// </summary>
        ///// <param name="type"></param>
        ///// <returns></returns>
        //private Object ReadCsType(Type type, Group cache, ushort key)
        //{
        //    var obj = Activator.CreateInstance(type);
        //    if (key != 0)
        //        cache.Add(key, obj);
        //    foreach (var property in type.GetProperties(BindingFlags.Public | BindingFlags.Instance | BindingFlags.SetProperty))
        //    {
        //        if ((property.PropertyType.IsGenericType && property.PropertyType.GetGenericTypeDefinition() == typeof(EntityReference<>)) ||
        //            property.PropertyType == typeof(EntityState) ||
        //            property.PropertyType == typeof(EntityKey) ||
        //            !property.CanWrite)
        //        {
        //            continue;
        //        }
        //        object value;
        //        Read(property.PropertyType, out value, cache);
        //        property.SetValue(obj, value);
        //    }
        //    return obj;
        //}

        ///// <summary>
        /////     Fallback serialization for unknown types.
        ///// </summary>
        ///// <param name="type"></param>
        ///// <param name="obj"></param>
        //private void WriteCsType(Type type, object obj, Group cache)
        //{
        //    foreach (var property in type.GetProperties(BindingFlags.Public | BindingFlags.Instance | BindingFlags.SetProperty))
        //    {
        //        if ((property.PropertyType.IsGenericType && property.PropertyType.GetGenericTypeDefinition() == typeof(EntityReference<>)) ||
        //            property.PropertyType == typeof(EntityState) ||
        //            property.PropertyType == typeof(EntityKey) ||
        //            !property.CanWrite)
        //        {
        //            continue;
        //        }
        //        Write(property.PropertyType, property.GetValue(obj), cache);
        //    }
        //}

        /// <summary>
        ///     Resizes the internal buffer array.
        /// </summary>
        /// <param name="newLength">The new length of the buffer.</param>
        private void Resize(long newLength)
        {
            var buffer = Buffer;
            Array.Resize(ref buffer, (int)newLength);
            Buffer = buffer;
        }

        public byte[] Buffer { get; private set; }
        public long Offset { get; private set; }
        public long Length { get; private set; }
        public uint ProtocolVersion { get; set; }

        public MessageFactory Factory { get; private set; }

        private static ConcurrentDictionary<Type, SortedList<int, Tuple<PropertyInfo, Type>>> s_typePropertyDic =
            new ConcurrentDictionary<Type, SortedList<int, Tuple<PropertyInfo, Type>>>();

        private bool _writeLength = true;
    }
}
