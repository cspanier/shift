using System;

namespace Shift.Protogen
{
    public interface IVariant
    {
        int Which();

        Type GetNthType(int which);

        object GetValue();

        void SetValue(object value);
    }

    public class EmptyType : IVariant
    {
        public int Which()
        {
            throw new NotImplementedException();
        }

        public Type GetNthType(int which)
        {
            throw new NotImplementedException();
        }

        public object GetValue()
        {
            throw new NotImplementedException();
        }

        public void SetValue(object value)
        {
            throw new NotImplementedException();
        }
    }

    /// <summary>
    /// A variant type that can only store values of types T1 through TRest.
    /// </summary>
    /// <remarks>
    /// The cost of this type is that basic types are boxed to objects.
    /// </remarks>
    /// <typeparam name="T1"></typeparam>
    /// <typeparam name="T2"></typeparam>
    /// <typeparam name="T3"></typeparam>
    /// <typeparam name="T4"></typeparam>
    /// <typeparam name="T5"></typeparam>
    /// <typeparam name="T6"></typeparam>
    /// <typeparam name="T7"></typeparam>
    /// <typeparam name="TRest"></typeparam>
    public class Variant<T1, T2, T3, T4, T5, T6, T7, TRest> :
        IVariant
        where TRest : IVariant, new()
    {
        public Variant()
        {
            if (typeof(TRest) != typeof(EmptyType))
                Rest = new TRest();
        }

        public Variant(Variant<T1, T2, T3, T4, T5, T6, T7, TRest> other)
        {
            _value = other._value;
        }

        public Variant(object value)
        {
            if (typeof(TRest) != typeof(EmptyType))
                Rest = new TRest();
            Value = value;
        }

        public Variant(T1 value) { _value = value; }
        public Variant(T2 value) { _value = value; }
        public Variant(T3 value) { _value = value; }
        public Variant(T4 value) { _value = value; }
        public Variant(T5 value) { _value = value; }
        public Variant(T6 value) { _value = value; }
        public Variant(T7 value) { _value = value; }

        public int Which()
        {
            if (Value is T1)
                return 0;
            else if (Value is T2)
                return 1;
            else if (Value is T3)
                return 2;
            else if (Value is T4)
                return 3;
            else if (Value is T5)
                return 4;
            else if (Value is T6)
                return 5;
            else if (Value is T7)
                return 6;
            else if (Rest != null)
                return Rest.Which() + 7;
            else
                throw new IndexOutOfRangeException();
        }

        public Type GetNthType(int which)
        {
            if (which == 0)
                return typeof(T1);
            else if (which == 1)
                return typeof(T2);
            else if (which == 2)
                return typeof(T3);
            else if (which == 3)
                return typeof(T4);
            else if (which == 4)
                return typeof(T5);
            else if (which == 5)
                return typeof(T6);
            else if (which == 6)
                return typeof(T7);
            else if (Rest != null)
                return Rest.GetNthType(which - 7);
            else
                throw new IndexOutOfRangeException();
        }

        public bool Is<T>()
        {
            return Value is T;
        }

        public object GetValue()
        {
            return Value;
        }

        public T GetValue<T>()
        {
            var value = Value;
            if (value is T)
                return (T)value;
            else
            {
                string msg = string.Format("Cannot convert \"{0}\" to \"{1}\"", value != null ? value.GetType().Name : "???", typeof(T).Name);
                throw new InvalidCastException(msg);
            }
        }

        public bool TryGetValue<T>(out T result)
        {
            var value = Value;
            if (value is T)
            {
                result = (T)value;
                return true;
            }
            else
            {
                result = default(T);
                return false;
            }
        }

        public void SetValue(object value)
        {
            Value = value;
        }

        public void SetValue<T>(T value)
        {
            Value = value;
        }

        public static implicit operator T1(Variant<T1, T2, T3, T4, T5, T6, T7, TRest> variant)
        {
            if (!(variant.Value is T1))
            {
                throw new InvalidCastException("Cannot convert a variant of type " +
                    variant.Value.GetType().FullName + " to " + typeof(T1).FullName + ".");
            }
            return (T1)variant.Value;
        }

        public static implicit operator T2(Variant<T1, T2, T3, T4, T5, T6, T7, TRest> variant)
        {
            if (!(variant.Value is T2))
            {
                throw new InvalidCastException("Cannot convert a variant of type " +
                    variant.Value.GetType().FullName + " to " + typeof(T2).FullName + ".");
            }
            return (T2)variant.Value;
        }

        public static implicit operator T3(Variant<T1, T2, T3, T4, T5, T6, T7, TRest> variant)
        {
            if (!(variant.Value is T3))
            {
                throw new InvalidCastException("Cannot convert a variant of type " +
                    variant.Value.GetType().FullName + " to " + typeof(T3).FullName + ".");
            }
            return (T3)variant.Value;
        }

        public static implicit operator T4(Variant<T1, T2, T3, T4, T5, T6, T7, TRest> variant)
        {
            if (!(variant.Value is T4))
            {
                throw new InvalidCastException("Cannot convert a variant of type " +
                    variant.Value.GetType().FullName + " to " + typeof(T4).FullName + ".");
            }
            return (T4)variant.Value;
        }

        public static implicit operator T5(Variant<T1, T2, T3, T4, T5, T6, T7, TRest> variant)
        {
            if (!(variant.Value is T5))
            {
                throw new InvalidCastException("Cannot convert a variant of type " +
                    variant.Value.GetType().FullName + " to " + typeof(T5).FullName + ".");
            }
            return (T5)variant.Value;
        }

        public static implicit operator T6(Variant<T1, T2, T3, T4, T5, T6, T7, TRest> variant)
        {
            if (!(variant.Value is T6))
            {
                throw new InvalidCastException("Cannot convert a variant of type " +
                    variant.Value.GetType().FullName + " to " + typeof(T6).FullName + ".");
            }
            return (T6)variant.Value;
        }

        public static implicit operator T7(Variant<T1, T2, T3, T4, T5, T6, T7, TRest> variant)
        {
            if (!(variant.Value is T7))
            {
                throw new InvalidCastException("Cannot convert a variant of type " +
                    variant.Value.GetType().FullName + " to " + typeof(T7).FullName + ".");
            }
            return (T7)variant.Value;
        }

        public Type Type1 { get { return typeof(T1); } }
        public Type Type2 { get { return typeof(T2); } }
        public Type Type3 { get { return typeof(T3); } }
        public Type Type4 { get { return typeof(T4); } }
        public Type Type5 { get { return typeof(T5); } }
        public Type Type6 { get { return typeof(T6); } }
        public Type Type7 { get { return typeof(T7); } }
        public TRest Rest { get; private set; }

        public object Value
        {
            get
            {
                if (_value == null && Rest != null)
                    return Rest.GetValue();
                return _value;
            }
            set
            {
                if (value == null)
                {
                    _value = value;
                    if (Rest != null)
                        Rest.SetValue(null);
                }
                else if (value is T1 || value is T2 || value is T3 || value is T4 ||
                    value is T5 || value is T6 || value is T7)
                {
                    _value = value;
                    if (Rest != null)
                        Rest.SetValue(null);
                }
                else if (Rest != null)
                {
                    _value = null;
                    Rest.SetValue(value);
                }
                else
                    throw new ArgumentException();
            }
        }
        private object _value;
    }

    public class Variant<T1, T2, T3, T4, T5, T6, T7> :
        Variant<T1, T2, T3, T4, T5, T6, T7, EmptyType>
    {
        public Variant()
        {
        }

        public Variant(object value)
            : base(value)
        {
        }

        public Variant(Variant<T1, T2, T3, T4, T5, T6, T7> other)
            : base(other.Value)
        {
        }

        public Variant(T1 value) : base(value) { }
        public Variant(T2 value) : base(value) { }
        public Variant(T3 value) : base(value) { }
        public Variant(T4 value) : base(value) { }
        public Variant(T5 value) : base(value) { }
        public Variant(T6 value) : base(value) { }
        public Variant(T7 value) : base(value) { }

        public static implicit operator T1(Variant<T1, T2, T3, T4, T5, T6, T7> variant)
        {
            if (!(variant.Value is T1))
            {
                throw new InvalidCastException("Cannot convert a variant of type " +
                    variant.Value.GetType().FullName + " to " + typeof(T1).FullName + ".");
            }
            return (T1)variant.Value;
        }

        public static implicit operator T2(Variant<T1, T2, T3, T4, T5, T6, T7> variant)
        {
            if (!(variant.Value is T2))
            {
                throw new InvalidCastException("Cannot convert a variant of type " +
                    variant.Value.GetType().FullName + " to " + typeof(T2).FullName + ".");
            }
            return (T2)variant.Value;
        }

        public static implicit operator T3(Variant<T1, T2, T3, T4, T5, T6, T7> variant)
        {
            if (!(variant.Value is T3))
            {
                throw new InvalidCastException("Cannot convert a variant of type " +
                    variant.Value.GetType().FullName + " to " + typeof(T3).FullName + ".");
            }
            return (T3)variant.Value;
        }

        public static implicit operator T4(Variant<T1, T2, T3, T4, T5, T6, T7> variant)
        {
            if (!(variant.Value is T4))
            {
                throw new InvalidCastException("Cannot convert a variant of type " +
                    variant.Value.GetType().FullName + " to " + typeof(T4).FullName + ".");
            }
            return (T4)variant.Value;
        }

        public static implicit operator T5(Variant<T1, T2, T3, T4, T5, T6, T7> variant)
        {
            if (!(variant.Value is T5))
            {
                throw new InvalidCastException("Cannot convert a variant of type " +
                    variant.Value.GetType().FullName + " to " + typeof(T5).FullName + ".");
            }
            return (T5)variant.Value;
        }

        public static implicit operator T6(Variant<T1, T2, T3, T4, T5, T6, T7> variant)
        {
            if (!(variant.Value is T6))
            {
                throw new InvalidCastException("Cannot convert a variant of type " +
                    variant.Value.GetType().FullName + " to " + typeof(T6).FullName + ".");
            }
            return (T6)variant.Value;
        }

        public static implicit operator T7(Variant<T1, T2, T3, T4, T5, T6, T7> variant)
        {
            if (!(variant.Value is T7))
            {
                throw new InvalidCastException("Cannot convert a variant of type " +
                    variant.Value.GetType().FullName + " to " + typeof(T7).FullName + ".");
            }
            return (T7)variant.Value;
        }
    }

    public class Variant<T1, T2, T3, T4, T5, T6> :
        Variant<T1, T2, T3, T4, T5, T6, EmptyType, EmptyType>
    {
        public Variant()
        {
        }

        public Variant(object value)
            : base(value)
        {
        }

        public Variant(Variant<T1, T2, T3, T4, T5, T6> other)
            : base(other.Value)
        {
        }

        public Variant(T1 value) : base(value) { }
        public Variant(T2 value) : base(value) { }
        public Variant(T3 value) : base(value) { }
        public Variant(T4 value) : base(value) { }
        public Variant(T5 value) : base(value) { }
        public Variant(T6 value) : base(value) { }

        public static implicit operator T1(Variant<T1, T2, T3, T4, T5, T6> variant)
        {
            if (!(variant.Value is T1))
            {
                throw new InvalidCastException("Cannot convert a variant of type " +
                    variant.Value.GetType().FullName + " to " + typeof(T1).FullName + ".");
            }
            return (T1)variant.Value;
        }

        public static implicit operator T2(Variant<T1, T2, T3, T4, T5, T6> variant)
        {
            if (!(variant.Value is T2))
            {
                throw new InvalidCastException("Cannot convert a variant of type " +
                    variant.Value.GetType().FullName + " to " + typeof(T2).FullName + ".");
            }
            return (T2)variant.Value;
        }

        public static implicit operator T3(Variant<T1, T2, T3, T4, T5, T6> variant)
        {
            if (!(variant.Value is T3))
            {
                throw new InvalidCastException("Cannot convert a variant of type " +
                    variant.Value.GetType().FullName + " to " + typeof(T3).FullName + ".");
            }
            return (T3)variant.Value;
        }

        public static implicit operator T4(Variant<T1, T2, T3, T4, T5, T6> variant)
        {
            if (!(variant.Value is T4))
            {
                throw new InvalidCastException("Cannot convert a variant of type " +
                    variant.Value.GetType().FullName + " to " + typeof(T4).FullName + ".");
            }
            return (T4)variant.Value;
        }

        public static implicit operator T5(Variant<T1, T2, T3, T4, T5, T6> variant)
        {
            if (!(variant.Value is T5))
            {
                throw new InvalidCastException("Cannot convert a variant of type " +
                    variant.Value.GetType().FullName + " to " + typeof(T5).FullName + ".");
            }
            return (T5)variant.Value;
        }

        public static implicit operator T6(Variant<T1, T2, T3, T4, T5, T6> variant)
        {
            if (!(variant.Value is T6))
            {
                throw new InvalidCastException("Cannot convert a variant of type " +
                    variant.Value.GetType().FullName + " to " + typeof(T6).FullName + ".");
            }
            return (T6)variant.Value;
        }
    }

    public class Variant<T1, T2, T3, T4, T5> :
        Variant<T1, T2, T3, T4, T5, EmptyType, EmptyType, EmptyType>
    {
        public Variant()
        {
        }

        public Variant(object value)
            : base(value)
        {
        }

        public Variant(Variant<T1, T2, T3, T4, T5> other)
            : base(other.Value)
        {
        }

        public Variant(T1 value) : base(value) { }
        public Variant(T2 value) : base(value) { }
        public Variant(T3 value) : base(value) { }
        public Variant(T4 value) : base(value) { }
        public Variant(T5 value) : base(value) { }

        public static implicit operator T1(Variant<T1, T2, T3, T4, T5> variant)
        {
            if (!(variant.Value is T1))
            {
                throw new InvalidCastException("Cannot convert a variant of type " +
                    variant.Value.GetType().FullName + " to " + typeof(T1).FullName + ".");
            }
            return (T1)variant.Value;
        }

        public static implicit operator T2(Variant<T1, T2, T3, T4, T5> variant)
        {
            if (!(variant.Value is T2))
            {
                throw new InvalidCastException("Cannot convert a variant of type " +
                    variant.Value.GetType().FullName + " to " + typeof(T2).FullName + ".");
            }
            return (T2)variant.Value;
        }

        public static implicit operator T3(Variant<T1, T2, T3, T4, T5> variant)
        {
            if (!(variant.Value is T3))
            {
                throw new InvalidCastException("Cannot convert a variant of type " +
                    variant.Value.GetType().FullName + " to " + typeof(T3).FullName + ".");
            }
            return (T3)variant.Value;
        }

        public static implicit operator T4(Variant<T1, T2, T3, T4, T5> variant)
        {
            if (!(variant.Value is T4))
            {
                throw new InvalidCastException("Cannot convert a variant of type " +
                    variant.Value.GetType().FullName + " to " + typeof(T4).FullName + ".");
            }
            return (T4)variant.Value;
        }

        public static implicit operator T5(Variant<T1, T2, T3, T4, T5> variant)
        {
            if (!(variant.Value is T5))
            {
                throw new InvalidCastException("Cannot convert a variant of type " +
                    variant.Value.GetType().FullName + " to " + typeof(T5).FullName + ".");
            }
            return (T5)variant.Value;
        }
    }

    public class Variant<T1, T2, T3, T4> :
        Variant<T1, T2, T3, T4, EmptyType, EmptyType, EmptyType, EmptyType>
    {
        public Variant()
        {
        }

        public Variant(object value)
            : base(value)
        {
        }

        public Variant(Variant<T1, T2, T3, T4> other)
            : base(other.Value)
        {
        }

        public Variant(T1 value) : base(value) { }
        public Variant(T2 value) : base(value) { }
        public Variant(T3 value) : base(value) { }
        public Variant(T4 value) : base(value) { }

        public static implicit operator T1(Variant<T1, T2, T3, T4> variant)
        {
            if (!(variant.Value is T1))
            {
                throw new InvalidCastException("Cannot convert a variant of type " +
                    variant.Value.GetType().FullName + " to " + typeof(T1).FullName + ".");
            }
            return (T1)variant.Value;
        }

        public static implicit operator T2(Variant<T1, T2, T3, T4> variant)
        {
            if (!(variant.Value is T2))
            {
                throw new InvalidCastException("Cannot convert a variant of type " +
                    variant.Value.GetType().FullName + " to " + typeof(T2).FullName + ".");
            }
            return (T2)variant.Value;
        }

        public static implicit operator T3(Variant<T1, T2, T3, T4> variant)
        {
            if (!(variant.Value is T3))
            {
                throw new InvalidCastException("Cannot convert a variant of type " +
                    variant.Value.GetType().FullName + " to " + typeof(T3).FullName + ".");
            }
            return (T3)variant.Value;
        }

        public static implicit operator T4(Variant<T1, T2, T3, T4> variant)
        {
            if (!(variant.Value is T4))
            {
                throw new InvalidCastException("Cannot convert a variant of type " +
                    variant.Value.GetType().FullName + " to " + typeof(T4).FullName + ".");
            }
            return (T4)variant.Value;
        }
    }

    public class Variant<T1, T2, T3> :
        Variant<T1, T2, T3, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType>
    {
        public Variant()
        {
        }

        public Variant(object value)
            : base(value)
        {
        }

        public Variant(Variant<T1, T2, T3> other)
            : base(other.Value)
        {
        }

        public Variant(T1 value) : base(value) { }
        public Variant(T2 value) : base(value) { }
        public Variant(T3 value) : base(value) { }

        public static implicit operator T1(Variant<T1, T2, T3> variant)
        {
            if (!(variant.Value is T1))
            {
                throw new InvalidCastException("Cannot convert a variant of type " +
                    variant.Value.GetType().FullName + " to " + typeof(T1).FullName + ".");
            }
            return (T1)variant.Value;
        }

        public static implicit operator T2(Variant<T1, T2, T3> variant)
        {
            if (!(variant.Value is T2))
            {
                throw new InvalidCastException("Cannot convert a variant of type " +
                    variant.Value.GetType().FullName + " to " + typeof(T2).FullName + ".");
            }
            return (T2)variant.Value;
        }

        public static implicit operator T3(Variant<T1, T2, T3> variant)
        {
            if (!(variant.Value is T3))
            {
                throw new InvalidCastException("Cannot convert a variant of type " +
                    variant.Value.GetType().FullName + " to " + typeof(T3).FullName + ".");
            }
            return (T3)variant.Value;
        }
    }

    public class Variant<T1, T2> :
        Variant<T1, T2, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType>
    {
        public Variant()
        {
        }

        public Variant(object value)
            : base(value)
        {
        }

        public Variant(Variant<T1, T2> other)
            : base(other.Value)
        {
        }

        public Variant(T1 value) : base(value) { }
        public Variant(T2 value) : base(value) { }

        public static implicit operator T1(Variant<T1, T2> variant)
        {
            if (!(variant.Value is T1))
            {
                throw new InvalidCastException("Cannot convert a variant of type " +
                    variant.Value.GetType().FullName + " to " + typeof(T1).FullName + ".");
            }
            return (T1)variant.Value;
        }

        public static implicit operator T2(Variant<T1, T2> variant)
        {
            if (!(variant.Value is T2))
            {
                throw new InvalidCastException("Cannot convert a variant of type " +
                    variant.Value.GetType().FullName + " to " + typeof(T2).FullName + ".");
            }
            return (T2)variant.Value;
        }
    }

    public class Variant<T1> :
        Variant<T1, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType>
    {
        public Variant()
        {
        }

        public Variant(object value)
            : base(value)
        {
        }

        public Variant(Variant<T1> other)
            : base(other.Value)
        {
        }

        public Variant(T1 value) : base(value) { }

        public static implicit operator T1(Variant<T1> variant)
        {
            if (!(variant.Value is T1))
            {
                throw new InvalidCastException("Cannot convert a variant of type " +
                    variant.Value.GetType().FullName + " to " + typeof(T1).FullName + ".");
            }
            return (T1)variant.Value;
        }
    }
}
