using System;
using System.Collections.Generic;

namespace Shift.Network
{
    /// <summary>
    ///     The Group is a support class used to serialize arbitrary graphs of objects
    ///     (i.e. graphs with circular dependencies). It provides a unique key for each
    ///     object in the group, which can be used for (de-)serialization.
    /// </summary>
    public class Group
    {
        public ushort Find(object obj)
        {
            if (obj == null)
                throw new ArgumentNullException();
            foreach (var pair in _cache)
            {
                if (pair.Value == obj)
                    return pair.Key;
            }
            return 0;
        }

        public object Find(ushort key)
        {
            if (key == 0)
                throw new ArgumentException();
            object result;
            if (_cache.TryGetValue(key, out result))
                return result;
            else
                return null;
        }

        public ushort Add(object obj)
        {
            if (obj == null)
                throw new ArgumentNullException();
            var key = ++_nextKey;
            _cache.Add(key, obj);
            return key;
        }

        public void Add(ushort key, object obj)
        {
            _cache.Add(key, obj);
        }

        private Dictionary<ushort, object> _cache =
            new Dictionary<ushort, object>();
        private ushort _nextKey = 0;
    }
}
