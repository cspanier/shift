using System;

namespace Shift.Network
{
    internal static class TypeExtension
    {
        internal static Type[] FundamentalTypes =
        {
            typeof(bool),
            typeof(char),
            typeof(sbyte),
            typeof(byte),
            typeof(short),
            typeof(ushort),
            typeof(int),
            typeof(uint),
            typeof(long),
            typeof(ulong),
            typeof(float),
            typeof(double)
        };

        /// <summary>
        /// Returns true if this Type matches any of a set of Types.
        /// </summary>
        /// <param name="type">This type.</param>
        /// <param name="types">The Types to compare this Type to.</param>
        public static bool In(this Type type, params Type[] types)
        {
            foreach (Type t in types)
            {
#if WINDOWS_UWP
                if (t.GetTypeInfo().IsAssignableFrom(type.GetTypeInfo()))
#else
                if (t.IsAssignableFrom(type))
#endif
                    return true;
            }
            return false;
        }

        /// <summary>
        /// Returns true if this Type is one of the fundamental types.
        /// </summary>
        public static bool IsFundamental(this Type type)
        {
            return type.In(FundamentalTypes);
        }
    }
}
