using System;

namespace Thales.Sagittarius.Data.Media3D
{
    public struct Point3D : IFormattable
    {
        internal double _x;
        internal double _y;
        internal double _z;
        public Point3D(double x, double y, double z)
        {
            this._x = x;
            this._y = y;
            this._z = z;
        }

        public void Offset(double offsetX, double offsetY, double offsetZ)
        {
            this._x += offsetX;
            this._y += offsetY;
            this._z += offsetZ;
        }

        public static Point3D operator +(Point3D point, Vector3D vector) =>
            new Point3D(point._x + vector._x, point._y + vector._y, point._z + vector._z);

        public static Point3D Add(Point3D point, Vector3D vector) =>
            new Point3D(point._x + vector._x, point._y + vector._y, point._z + vector._z);

        public static Point3D operator -(Point3D point, Vector3D vector) =>
            new Point3D(point._x - vector._x, point._y - vector._y, point._z - vector._z);

        public static Point3D Subtract(Point3D point, Vector3D vector) =>
            new Point3D(point._x - vector._x, point._y - vector._y, point._z - vector._z);

        public static Vector3D operator -(Point3D point1, Point3D point2) =>
            new Vector3D(point1._x - point2._x, point1._y - point2._y, point1._z - point2._z);

        public static Vector3D Subtract(Point3D point1, Point3D point2)
        {
            Vector3D result = new Vector3D();
            Subtract(ref point1, ref point2, out result);
            return result;
        }

        internal static void Subtract(ref Point3D p1, ref Point3D p2, out Vector3D result)
        {
            result._x = p1._x - p2._x;
            result._y = p1._y - p2._y;
            result._z = p1._z - p2._z;
        }

        public static Point3D operator *(Point3D point, Matrix3D matrix) =>
            matrix.Transform(point);

        public static Point3D Multiply(Point3D point, Matrix3D matrix) =>
            matrix.Transform(point);

        public static explicit operator Vector3D(Point3D point) =>
            new Vector3D(point._x, point._y, point._z);

        public static explicit operator Point4D(Point3D point) =>
            new Point4D(point._x, point._y, point._z, 1.0);

        public static bool operator ==(Point3D point1, Point3D point2) =>
            (((point1.X == point2.X) && (point1.Y == point2.Y)) && (point1.Z == point2.Z));

        public static bool operator !=(Point3D point1, Point3D point2) =>
            !(point1 == point2);

        public static bool Equals(Point3D point1, Point3D point2) =>
            ((point1.X.Equals(point2.X) && point1.Y.Equals(point2.Y)) && point1.Z.Equals(point2.Z));

        public override bool Equals(object o)
        {
            if ((o == null) || !(o is Point3D))
            {
                return false;
            }
            Point3D pointd = (Point3D)o;
            return Equals(this, pointd);
        }

        public bool Equals(Point3D value) =>
            Equals(this, value);

        public override int GetHashCode() =>
            ((this.X.GetHashCode() ^ this.Y.GetHashCode()) ^ this.Z.GetHashCode());

        public double X
        {
            get
            {
                return this._x;
            }
            set
            {
                this._x = value;
            }
        }

        public double Y
        {
            get
            {
                return this._y;
            }
            set
            {
                this._y = value;
            }
        }
        public double Z
        {
            get
            {
                return this._z;
            }
            set
            {
                this._z = value;
            }
        }

        public override string ToString() =>
            this.ConvertToString(null, null);

        public string ToString(IFormatProvider provider) =>
            this.ConvertToString(null, provider);

        string IFormattable.ToString(string format, IFormatProvider provider) =>
            this.ConvertToString(format, provider);

        internal string ConvertToString(string format, IFormatProvider provider)
        {
            char numericListSeparator = TokenizerHelper.GetNumericListSeparator(provider);
            string[] textArray1 = new string[] { "{1:", format, "}{0}{2:", format, "}{0}{3:", format, "}" };
            object[] args = new object[] { numericListSeparator, this._x, this._y, this._z };
            return string.Format(provider, string.Concat(textArray1), args);
        }
    }
}
