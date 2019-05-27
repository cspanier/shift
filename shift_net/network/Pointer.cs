namespace Shift.Network
{
  /// <summary>
  /// 
  /// </summary>
  public interface IPointer
  {
  }

  /// <summary>
  /// 
  /// </summary>
  /// <typeparam name="T"></typeparam>
  public struct RawPtr<T> : IPointer
  {
  }

  /// <summary>
  /// 
  /// </summary>
  /// <typeparam name="T"></typeparam>
  public struct UniquePtr<T> : IPointer
  {
  }

  /// <summary>
  /// 
  /// </summary>
  /// <typeparam name="T"></typeparam>
  public struct SharedPtr<T> : IPointer
  {
  }

  /// <summary>
  /// 
  /// </summary>
  /// <typeparam name="T"></typeparam>
  public struct WeakPtr<T> : IPointer
  {
  }

  /// <summary>
  /// 
  /// </summary>
  /// <typeparam name="T"></typeparam>
  public struct GroupPtr<T> : IPointer
  {
  }
}
