using System;

namespace Thales.Sagittarius.Data.Media3D
{
    public struct Vector3D : IFormattable
    {
        internal double _x;
        internal double _y;
        internal double _z;
        public Vector3D(double x, double y, double z)
        {
            this._x = x;
            this._y = y;
            this._z = z;
        }

        public double Length =>
            Math.Sqrt(((this._x * this._x) + (this._y * this._y)) + (this._z * this._z));
        public double LengthSquared =>
            (((this._x * this._x) + (this._y * this._y)) + (this._z * this._z));
        public void Normalize()
        {
            double num = Math.Abs(this._x);
            double num3 = Math.Abs(this._y);
            double num2 = Math.Abs(this._z);
            if (num3 > num)
            {
                num = num3;
            }
            if (num2 > num)
            {
                num = num2;
            }
            this._x /= num;
            this._y /= num;
            this._z /= num;
            double num4 = Math.Sqrt(((this._x * this._x) + (this._y * this._y)) + (this._z * this._z));
            this = (Vector3D)(this / num4);
        }

        public static double AngleBetween(Vector3D vector1, Vector3D vector2)
        {
            double num;
            Vector3D vectord;
            vector1.Normalize();
            vector2.Normalize();
            if (DotProduct(vector1, vector2) < 0.0)
            {
                vectord = -vector1 - vector2;
                num = 3.1415926535897931 - (2.0 * Math.Asin(vectord.Length / 2.0));
            }
            else
            {
                vectord = vector1 - vector2;
                num = 2.0 * Math.Asin(vectord.Length / 2.0);
            }
            return RadiansToDegrees(num);
        }

        public static Vector3D operator -(Vector3D vector) =>
            new Vector3D(-vector._x, -vector._y, -vector._z);

        public void Negate()
        {
            this._x = -this._x;
            this._y = -this._y;
            this._z = -this._z;
        }

        public static Vector3D operator +(Vector3D vector1, Vector3D vector2) =>
            new Vector3D(vector1._x + vector2._x, vector1._y + vector2._y, vector1._z + vector2._z);

        public static Vector3D Add(Vector3D vector1, Vector3D vector2) =>
            new Vector3D(vector1._x + vector2._x, vector1._y + vector2._y, vector1._z + vector2._z);

        public static Vector3D operator -(Vector3D vector1, Vector3D vector2) =>
            new Vector3D(vector1._x - vector2._x, vector1._y - vector2._y, vector1._z - vector2._z);

        public static Vector3D Subtract(Vector3D vector1, Vector3D vector2) =>
            new Vector3D(vector1._x - vector2._x, vector1._y - vector2._y, vector1._z - vector2._z);

        public static Point3D operator +(Vector3D vector, Point3D point) =>
            new Point3D(vector._x + point._x, vector._y + point._y, vector._z + point._z);

        public static Point3D Add(Vector3D vector, Point3D point) =>
            new Point3D(vector._x + point._x, vector._y + point._y, vector._z + point._z);

        public static Point3D operator -(Vector3D vector, Point3D point) =>
            new Point3D(vector._x - point._x, vector._y - point._y, vector._z - point._z);

        public static Point3D Subtract(Vector3D vector, Point3D point) =>
            new Point3D(vector._x - point._x, vector._y - point._y, vector._z - point._z);

        public static Vector3D operator *(Vector3D vector, double scalar) =>
            new Vector3D(vector._x * scalar, vector._y * scalar, vector._z * scalar);

        public static Vector3D Multiply(Vector3D vector, double scalar) =>
            new Vector3D(vector._x * scalar, vector._y * scalar, vector._z * scalar);

        public static Vector3D operator *(double scalar, Vector3D vector) =>
            new Vector3D(vector._x * scalar, vector._y * scalar, vector._z * scalar);

        public static Vector3D Multiply(double scalar, Vector3D vector) =>
            new Vector3D(vector._x * scalar, vector._y * scalar, vector._z * scalar);

        public static Vector3D operator /(Vector3D vector, double scalar) =>
            ((Vector3D)(vector * (1.0 / scalar)));

        public static Vector3D Divide(Vector3D vector, double scalar) =>
            ((Vector3D)(vector * (1.0 / scalar)));

        public static Vector3D operator *(Vector3D vector, Matrix3D matrix) =>
            matrix.Transform(vector);

        public static Vector3D Multiply(Vector3D vector, Matrix3D matrix) =>
            matrix.Transform(vector);

        public static double DotProduct(Vector3D vector1, Vector3D vector2) =>
            DotProduct(ref vector1, ref vector2);

        internal static double DotProduct(ref Vector3D vector1, ref Vector3D vector2) =>
            (((vector1._x * vector2._x) + (vector1._y * vector2._y)) + (vector1._z * vector2._z));

        public static Vector3D CrossProduct(Vector3D vector1, Vector3D vector2)
        {
            Vector3D vectord;
            CrossProduct(ref vector1, ref vector2, out vectord);
            return vectord;
        }

        internal static void CrossProduct(ref Vector3D vector1, ref Vector3D vector2, out Vector3D result)
        {
            result._x = (vector1._y * vector2._z) - (vector1._z * vector2._y);
            result._y = (vector1._z * vector2._x) - (vector1._x * vector2._z);
            result._z = (vector1._x * vector2._y) - (vector1._y * vector2._x);
        }

        public static explicit operator Point3D(Vector3D vector) =>
            new Point3D(vector._x, vector._y, vector._z);

        public static bool operator ==(Vector3D vector1, Vector3D vector2) =>
            (((vector1.X == vector2.X) && (vector1.Y == vector2.Y)) && (vector1.Z == vector2.Z));

        public static bool operator !=(Vector3D vector1, Vector3D vector2) =>
            !(vector1 == vector2);

        public static bool Equals(Vector3D vector1, Vector3D vector2) =>
            ((vector1.X.Equals(vector2.X) && vector1.Y.Equals(vector2.Y)) && vector1.Z.Equals(vector2.Z));

        public override bool Equals(object o)
        {
            if ((o == null) || !(o is Vector3D))
            {
                return false;
            }
            Vector3D vectord = (Vector3D)o;
            return Equals(this, vectord);
        }

        public bool Equals(Vector3D value) =>
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
        internal static double RadiansToDegrees(double radians) =>
           (radians * 57.295779513082323);
    }
}
