using System.Collections.Generic;

namespace Shift.Network
{
    public class MessageFactory
    {
        public delegate Message CreateMessageDelegate();

        public void Register(uint uid, CreateMessageDelegate factory)
        {
            _factories.Add(uid, factory);
        }

        public Message Create(uint uid)
        {
            CreateMessageDelegate factory;
            if (!_factories.TryGetValue(uid, out factory))
                return null;
            return factory();
        }

        private Dictionary<uint, CreateMessageDelegate> _factories =
            new Dictionary<uint, CreateMessageDelegate>();
    }
}
