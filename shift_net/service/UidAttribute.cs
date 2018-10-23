using System;

namespace Shift.Service
{
    [System.AttributeUsage(
        System.AttributeTargets.Class |
        System.AttributeTargets.Struct)]
    public class UidAttribute : Attribute
    {
    }
}
