using System;
using System.ComponentModel;

namespace Shift.Protogen
{
  [AttributeUsage(AttributeTargets.Property | AttributeTargets.Field,
    Inherited = true, AllowMultiple = false)]
#if !WINDOWS_UWP
  [ImmutableObject(true)]
#endif
  public sealed class OrderAttribute : Attribute
  {
    public OrderAttribute(int order)
    {
      _order = order;
    }

    public int Order
    {
      get { return _order; }
    }

    private readonly int _order;
  }
}
