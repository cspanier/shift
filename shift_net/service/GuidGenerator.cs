using System;

#if !WINDOWS_UWP
using System.Security.Cryptography;
#endif

namespace Shift.Service
{
    /// <summary>
    ///     A simple thread-safe 64bit random number generator used for
    ///     service and host Guids.
    /// </summary>
    /// <returns></returns>
    internal class GuidGenerator
    {
#if !WINDOWS_UWP
        public static ulong generate()
        {
            lock (s_random)
            {
                var data = new byte[sizeof(long)];
                s_random.Value.GetBytes(data);
                return BitConverter.ToUInt64(data, 0);
            }
        }

        private static readonly Lazy<RNGCryptoServiceProvider> s_random =
            new Lazy<RNGCryptoServiceProvider>(() => new RNGCryptoServiceProvider());
#else
        public static ulong generate()
        {
            byte[] guidArray = Guid.NewGuid().ToByteArray();
            var data = new byte[sizeof(long)];

            for (int i = 0; i < sizeof(long); i++)
            {
                data[i] = guidArray[i];
            }

            //_random.Value.GetBytes(data);
            return BitConverter.ToUInt64(data, 0);
        }
#endif

    }
}
