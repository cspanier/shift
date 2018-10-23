using System;
using System.ComponentModel;

namespace Shift.Protogen
{
    [AttributeUsage(AttributeTargets.Property | AttributeTargets.Field,
        Inherited = true, AllowMultiple = false)]
#if !WINDOWS_UWP
    [ImmutableObject(true)]
#endif
    public sealed class ProtoTypeAttribute : Attribute
    {
        public ProtoTypeAttribute(Type type)
        {
            _protoType = type;
        }

        public Type ProtoType
        {
            get { return _protoType; }
        }

        private readonly Type _protoType;
    }
}
