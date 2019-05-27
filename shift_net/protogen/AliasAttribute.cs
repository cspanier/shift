using System;
using System.ComponentModel;

namespace Shift.Protogen
{
  [AttributeUsage(AttributeTargets.Class | AttributeTargets.Struct,
    Inherited = true, AllowMultiple = false)]
#if !WINDOWS_UWP
  [ImmutableObject(true)]
#endif
  public sealed class AliasAttribute : Attribute
  {
    public AliasAttribute(Type aliasType)
    {
      _aliasType = aliasType;
    }

    public Type AliasType
    {
      get { return _aliasType; }
    }

    private readonly Type _aliasType;
  }
}
