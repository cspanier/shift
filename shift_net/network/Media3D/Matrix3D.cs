using System;

namespace Thales.Sagittarius.Data.Media3D
{
    public struct Matrix3D : IFormattable
    {
        private double _m11;
        private double _m12;
        private double _m13;
        private double _m14;
        private double _m21;
        private double _m22;
        private double _m23;
        private double _m24;
        private double _m31;
        private double _m32;
        private double _m33;
        private double _m34;
        private double _offsetX;
        private double _offsetY;
        private double _offsetZ;
        private double _m44;
        private bool _isNotKnownToBeIdentity;
        private static readonly Matrix3D s_identity;
        private const int c_identityHashCode = 0;
        public Matrix3D(double m11, double m12, double m13, double m14, double m21, double m22, double m23, double m24, double m31, double m32, double m33, double m34, double offsetX, double offsetY, double offsetZ, double m44)
        {
            _m11 = m11;
            _m12 = m12;
            _m13 = m13;
            _m14 = m14;
            _m21 = m21;
            _m22 = m22;
            _m23 = m23;
            _m24 = m24;
            _m31 = m31;
            _m32 = m32;
            _m33 = m33;
            _m34 = m34;
            _offsetX = offsetX;
            _offsetY = offsetY;
            _offsetZ = offsetZ;
            _m44 = m44;
            _isNotKnownToBeIdentity = true;
        }

        public static Matrix3D Identity =>
            s_identity;
        public void SetIdentity()
        {
            this = s_identity;
        }

        public bool IsIdentity
        {
            get
            {
                if (this.IsDistinguishedIdentity)
                {
                    return true;
                }
                if (((((_m11 == 1.0) && (_m12 == 0.0)) && ((_m13 == 0.0) && (_m14 == 0.0))) && (((_m21 == 0.0) && (_m22 == 1.0)) && ((_m23 == 0.0) && (_m24 == 0.0)))) && ((((_m31 == 0.0) && (_m32 == 0.0)) && ((_m33 == 1.0) && (_m34 == 0.0))) && (((_offsetX == 0.0) && (_offsetY == 0.0)) && ((_offsetZ == 0.0) && (_m44 == 1.0)))))
                {
                    this.IsDistinguishedIdentity = true;
                    return true;
                }
                return false;
            }
        }
        public void Prepend(Matrix3D matrix)
        {
            this = matrix * this;
        }

        public void Append(Matrix3D matrix)
        {
            this *= matrix;
        }

        public void Scale(Vector3D scale)
        {
            if (this.IsDistinguishedIdentity)
            {
                this.SetScaleMatrix(ref scale);
            }
            else
            {
                _m11 *= scale.X;
                _m12 *= scale.Y;
                _m13 *= scale.Z;
                _m21 *= scale.X;
                _m22 *= scale.Y;
                _m23 *= scale.Z;
                _m31 *= scale.X;
                _m32 *= scale.Y;
                _m33 *= scale.Z;
                _offsetX *= scale.X;
                _offsetY *= scale.Y;
                _offsetZ *= scale.Z;
            }
        }

        public void ScalePrepend(Vector3D scale)
        {
            if (this.IsDistinguishedIdentity)
            {
                this.SetScaleMatrix(ref scale);
            }
            else
            {
                _m11 *= scale.X;
                _m12 *= scale.X;
                _m13 *= scale.X;
                _m14 *= scale.X;
                _m21 *= scale.Y;
                _m22 *= scale.Y;
                _m23 *= scale.Y;
                _m24 *= scale.Y;
                _m31 *= scale.Z;
                _m32 *= scale.Z;
                _m33 *= scale.Z;
                _m34 *= scale.Z;
            }
        }

        public void ScaleAt(Vector3D scale, Point3D center)
        {
            if (this.IsDistinguishedIdentity)
            {
                this.SetScaleMatrix(ref scale, ref center);
            }
            else
            {
                double num = _m14 * center.X;
                _m11 = num + (scale.X * (_m11 - num));
                num = _m14 * center.Y;
                _m12 = num + (scale.Y * (_m12 - num));
                num = _m14 * center.Z;
                _m13 = num + (scale.Z * (_m13 - num));
                num = _m24 * center.X;
                _m21 = num + (scale.X * (_m21 - num));
                num = _m24 * center.Y;
                _m22 = num + (scale.Y * (_m22 - num));
                num = _m24 * center.Z;
                _m23 = num + (scale.Z * (_m23 - num));
                num = _m34 * center.X;
                _m31 = num + (scale.X * (_m31 - num));
                num = _m34 * center.Y;
                _m32 = num + (scale.Y * (_m32 - num));
                num = _m34 * center.Z;
                _m33 = num + (scale.Z * (_m33 - num));
                num = _m44 * center.X;
                _offsetX = num + (scale.X * (_offsetX - num));
                num = _m44 * center.Y;
                _offsetY = num + (scale.Y * (_offsetY - num));
                num = _m44 * center.Z;
                _offsetZ = num + (scale.Z * (_offsetZ - num));
            }
        }

        public void ScaleAtPrepend(Vector3D scale, Point3D center)
        {
            if (this.IsDistinguishedIdentity)
            {
                this.SetScaleMatrix(ref scale, ref center);
            }
            else
            {
                double num3 = center.X - (center.X * scale.X);
                double num2 = center.Y - (center.Y * scale.Y);
                double num = center.Z - (center.Z * scale.Z);
                _offsetX += ((_m11 * num3) + (_m21 * num2)) + (_m31 * num);
                _offsetY += ((_m12 * num3) + (_m22 * num2)) + (_m32 * num);
                _offsetZ += ((_m13 * num3) + (_m23 * num2)) + (_m33 * num);
                _m44 += ((_m14 * num3) + (_m24 * num2)) + (_m34 * num);
                _m11 *= scale.X;
                _m12 *= scale.X;
                _m13 *= scale.X;
                _m14 *= scale.X;
                _m21 *= scale.Y;
                _m22 *= scale.Y;
                _m23 *= scale.Y;
                _m24 *= scale.Y;
                _m31 *= scale.Z;
                _m32 *= scale.Z;
                _m33 *= scale.Z;
                _m34 *= scale.Z;
            }
        }

        public void Translate(Vector3D offset)
        {
            if (this.IsDistinguishedIdentity)
            {
                this.SetTranslationMatrix(ref offset);
            }
            else
            {
                _m11 += _m14 * offset.X;
                _m12 += _m14 * offset.Y;
                _m13 += _m14 * offset.Z;
                _m21 += _m24 * offset.X;
                _m22 += _m24 * offset.Y;
                _m23 += _m24 * offset.Z;
                _m31 += _m34 * offset.X;
                _m32 += _m34 * offset.Y;
                _m33 += _m34 * offset.Z;
                _offsetX += _m44 * offset.X;
                _offsetY += _m44 * offset.Y;
                _offsetZ += _m44 * offset.Z;
            }
        }

        public void TranslatePrepend(Vector3D offset)
        {
            if (this.IsDistinguishedIdentity)
            {
                this.SetTranslationMatrix(ref offset);
            }
            else
            {
                _offsetX += ((_m11 * offset.X) + (_m21 * offset.Y)) + (_m31 * offset.Z);
                _offsetY += ((_m12 * offset.X) + (_m22 * offset.Y)) + (_m32 * offset.Z);
                _offsetZ += ((_m13 * offset.X) + (_m23 * offset.Y)) + (_m33 * offset.Z);
                _m44 += ((_m14 * offset.X) + (_m24 * offset.Y)) + (_m34 * offset.Z);
            }
        }

        public static Matrix3D operator *(Matrix3D matrix1, Matrix3D matrix2)
        {
            if (matrix1.IsDistinguishedIdentity)
            {
                return matrix2;
            }
            if (matrix2.IsDistinguishedIdentity)
            {
                return matrix1;
            }
            return new Matrix3D((((matrix1._m11 * matrix2._m11) + (matrix1._m12 * matrix2._m21)) + (matrix1._m13 * matrix2._m31)) + (matrix1._m14 * matrix2._offsetX), (((matrix1._m11 * matrix2._m12) + (matrix1._m12 * matrix2._m22)) + (matrix1._m13 * matrix2._m32)) + (matrix1._m14 * matrix2._offsetY), (((matrix1._m11 * matrix2._m13) + (matrix1._m12 * matrix2._m23)) + (matrix1._m13 * matrix2._m33)) + (matrix1._m14 * matrix2._offsetZ), (((matrix1._m11 * matrix2._m14) + (matrix1._m12 * matrix2._m24)) + (matrix1._m13 * matrix2._m34)) + (matrix1._m14 * matrix2._m44), (((matrix1._m21 * matrix2._m11) + (matrix1._m22 * matrix2._m21)) + (matrix1._m23 * matrix2._m31)) + (matrix1._m24 * matrix2._offsetX), (((matrix1._m21 * matrix2._m12) + (matrix1._m22 * matrix2._m22)) + (matrix1._m23 * matrix2._m32)) + (matrix1._m24 * matrix2._offsetY), (((matrix1._m21 * matrix2._m13) + (matrix1._m22 * matrix2._m23)) + (matrix1._m23 * matrix2._m33)) + (matrix1._m24 * matrix2._offsetZ), (((matrix1._m21 * matrix2._m14) + (matrix1._m22 * matrix2._m24)) + (matrix1._m23 * matrix2._m34)) + (matrix1._m24 * matrix2._m44), (((matrix1._m31 * matrix2._m11) + (matrix1._m32 * matrix2._m21)) + (matrix1._m33 * matrix2._m31)) + (matrix1._m34 * matrix2._offsetX), (((matrix1._m31 * matrix2._m12) + (matrix1._m32 * matrix2._m22)) + (matrix1._m33 * matrix2._m32)) + (matrix1._m34 * matrix2._offsetY), (((matrix1._m31 * matrix2._m13) + (matrix1._m32 * matrix2._m23)) + (matrix1._m33 * matrix2._m33)) + (matrix1._m34 * matrix2._offsetZ), (((matrix1._m31 * matrix2._m14) + (matrix1._m32 * matrix2._m24)) + (matrix1._m33 * matrix2._m34)) + (matrix1._m34 * matrix2._m44), (((matrix1._offsetX * matrix2._m11) + (matrix1._offsetY * matrix2._m21)) + (matrix1._offsetZ * matrix2._m31)) + (matrix1._m44 * matrix2._offsetX), (((matrix1._offsetX * matrix2._m12) + (matrix1._offsetY * matrix2._m22)) + (matrix1._offsetZ * matrix2._m32)) + (matrix1._m44 * matrix2._offsetY), (((matrix1._offsetX * matrix2._m13) + (matrix1._offsetY * matrix2._m23)) + (matrix1._offsetZ * matrix2._m33)) + (matrix1._m44 * matrix2._offsetZ), (((matrix1._offsetX * matrix2._m14) + (matrix1._offsetY * matrix2._m24)) + (matrix1._offsetZ * matrix2._m34)) + (matrix1._m44 * matrix2._m44));
        }

        public static Matrix3D Multiply(Matrix3D matrix1, Matrix3D matrix2) =>
            (matrix1 * matrix2);

        public Point3D Transform(Point3D point)
        {
            this.MultiplyPoint(ref point);
            return point;
        }

        public void Transform(Point3D[] points)
        {
            if (points != null)
            {
                for (int i = 0; i < points.Length; i++)
                {
                    this.MultiplyPoint(ref points[i]);
                }
            }
        }

        public Point4D Transform(Point4D point)
        {
            this.MultiplyPoint(ref point);
            return point;
        }

        public void Transform(Point4D[] points)
        {
            if (points != null)
            {
                for (int i = 0; i < points.Length; i++)
                {
                    this.MultiplyPoint(ref points[i]);
                }
            }
        }

        public Vector3D Transform(Vector3D vector)
        {
            this.MultiplyVector(ref vector);
            return vector;
        }

        public void Transform(Vector3D[] vectors)
        {
            if (vectors != null)
            {
                for (int i = 0; i < vectors.Length; i++)
                {
                    this.MultiplyVector(ref vectors[i]);
                }
            }
        }

        public bool IsAffine =>
            (this.IsDistinguishedIdentity || ((((_m14 == 0.0) && (_m24 == 0.0)) && (_m34 == 0.0)) && (_m44 == 1.0)));
        public double Determinant
        {
            get
            {
                if (this.IsDistinguishedIdentity)
                {
                    return 1.0;
                }
                if (this.IsAffine)
                {
                    return this.GetNormalizedAffineDeterminant();
                }
                double num6 = (_m13 * _m24) - (_m23 * _m14);
                double num5 = (_m13 * _m34) - (_m33 * _m14);
                double num4 = (_m13 * _m44) - (_offsetZ * _m14);
                double num3 = (_m23 * _m34) - (_m33 * _m24);
                double num2 = (_m23 * _m44) - (_offsetZ * _m24);
                double num = (_m33 * _m44) - (_offsetZ * _m34);
                double num10 = ((_m22 * num5) - (_m32 * num6)) - (_m12 * num3);
                double num9 = ((_m12 * num2) - (_m22 * num4)) + (_offsetY * num6);
                double num8 = ((_m32 * num4) - (_offsetY * num5)) - (_m12 * num);
                double num7 = ((_m22 * num) - (_m32 * num2)) + (_offsetY * num3);
                return ((((_offsetX * num10) + (_m31 * num9)) + (_m21 * num8)) + (_m11 * num7));
            }
        }
        public bool HasInverse =>
            !IsZero(this.Determinant);
        public void Invert()
        {
            if (!this.InvertCore())
            {
                throw new InvalidOperationException("Matrix3D_NotInvertible", null);
            }
        }

        public double M11
        {
            get
            {
                if (this.IsDistinguishedIdentity)
                {
                    return 1.0;
                }
                return _m11;
            }
            set
            {
                if (this.IsDistinguishedIdentity)
                {
                    this = s_identity;
                    this.IsDistinguishedIdentity = false;
                }
                _m11 = value;
            }
        }
        public double M12
        {
            get
            {
                return _m12;
            }
            set
            {
                if (this.IsDistinguishedIdentity)
                {
                    this = s_identity;
                    this.IsDistinguishedIdentity = false;
                }
                _m12 = value;
            }
        }
        public double M13
        {
            get
            {
                return _m13;
            }
            set
            {
                if (this.IsDistinguishedIdentity)
                {
                    this = s_identity;
                    this.IsDistinguishedIdentity = false;
                }
                _m13 = value;
            }
        }
        public double M14
        {
            get
            {
                return _m14;
            }
            set
            {
                if (this.IsDistinguishedIdentity)
                {
                    this = s_identity;
                    this.IsDistinguishedIdentity = false;
                }
                _m14 = value;
            }
        }
        public double M21
        {
            get
            {
                return _m21;
            }
            set
            {
                if (this.IsDistinguishedIdentity)
                {
                    this = s_identity;
                    this.IsDistinguishedIdentity = false;
                }
                _m21 = value;
            }
        }
        public double M22
        {
            get
            {
                if (this.IsDistinguishedIdentity)
                {
                    return 1.0;
                }
                return _m22;
            }
            set
            {
                if (this.IsDistinguishedIdentity)
                {
                    this = s_identity;
                    this.IsDistinguishedIdentity = false;
                }
                _m22 = value;
            }
        }
        public double M23
        {
            get
            {
                return _m23;
            }
            set
            {
                if (this.IsDistinguishedIdentity)
                {
                    this = s_identity;
                    this.IsDistinguishedIdentity = false;
                }
                _m23 = value;
            }
        }
        public double M24
        {
            get
            {
                return _m24;
            }
            set
            {
                if (this.IsDistinguishedIdentity)
                {
                    this = s_identity;
                    this.IsDistinguishedIdentity = false;
                }
                _m24 = value;
            }
        }
        public double M31
        {
            get
            {
                return _m31;
            }
            set
            {
                if (this.IsDistinguishedIdentity)
                {
                    this = s_identity;
                    this.IsDistinguishedIdentity = false;
                }
                _m31 = value;
            }
        }
        public double M32
        {
            get
            {
                return _m32;
            }
            set
            {
                if (this.IsDistinguishedIdentity)
                {
                    this = s_identity;
                    this.IsDistinguishedIdentity = false;
                }
                _m32 = value;
            }
        }
        public double M33
        {
            get
            {
                if (this.IsDistinguishedIdentity)
                {
                    return 1.0;
                }
                return _m33;
            }
            set
            {
                if (this.IsDistinguishedIdentity)
                {
                    this = s_identity;
                    this.IsDistinguishedIdentity = false;
                }
                _m33 = value;
            }
        }
        public double M34
        {
            get
            {
                return _m34;
            }
            set
            {
                if (this.IsDistinguishedIdentity)
                {
                    this = s_identity;
                    this.IsDistinguishedIdentity = false;
                }
                _m34 = value;
            }
        }
        public double OffsetX
        {
            get
            {
                return _offsetX;
            }
            set
            {
                if (this.IsDistinguishedIdentity)
                {
                    this = s_identity;
                    this.IsDistinguishedIdentity = false;
                }
                _offsetX = value;
            }
        }
        public double OffsetY
        {
            get
            {
                return _offsetY;
            }
            set
            {
                if (this.IsDistinguishedIdentity)
                {
                    this = s_identity;
                    this.IsDistinguishedIdentity = false;
                }
                _offsetY = value;
            }
        }
        public double OffsetZ
        {
            get
            {
                return _offsetZ;
            }
            set
            {
                if (this.IsDistinguishedIdentity)
                {
                    this = s_identity;
                    this.IsDistinguishedIdentity = false;
                }
                _offsetZ = value;
            }
        }
        public double M44
        {
            get
            {
                if (this.IsDistinguishedIdentity)
                {
                    return 1.0;
                }
                return _m44;
            }
            set
            {
                if (this.IsDistinguishedIdentity)
                {
                    this = s_identity;
                    this.IsDistinguishedIdentity = false;
                }
                _m44 = value;
            }
        }
        internal void SetScaleMatrix(ref Vector3D scale)
        {
            _m11 = scale.X;
            _m22 = scale.Y;
            _m33 = scale.Z;
            _m44 = 1.0;
            this.IsDistinguishedIdentity = false;
        }

        internal void SetScaleMatrix(ref Vector3D scale, ref Point3D center)
        {
            _m11 = scale.X;
            _m22 = scale.Y;
            _m33 = scale.Z;
            _m44 = 1.0;
            _offsetX = center.X - (center.X * scale.X);
            _offsetY = center.Y - (center.Y * scale.Y);
            _offsetZ = center.Z - (center.Z * scale.Z);
            this.IsDistinguishedIdentity = false;
        }

        internal void SetTranslationMatrix(ref Vector3D offset)
        {
            _m11 = _m22 = _m33 = _m44 = 1.0;
            _offsetX = offset.X;
            _offsetY = offset.Y;
            _offsetZ = offset.Z;
            this.IsDistinguishedIdentity = false;
        }

        internal void MultiplyPoint(ref Point3D point)
        {
            if (!this.IsDistinguishedIdentity)
            {
                double x = point.X;
                double y = point.Y;
                double z = point.Z;
                point.X = (((x * _m11) + (y * _m21)) + (z * _m31)) + _offsetX;
                point.Y = (((x * _m12) + (y * _m22)) + (z * _m32)) + _offsetY;
                point.Z = (((x * _m13) + (y * _m23)) + (z * _m33)) + _offsetZ;
                if (!this.IsAffine)
                {
                    double num4 = (((x * _m14) + (y * _m24)) + (z * _m34)) + _m44;
                    point.X /= num4;
                    point.Y /= num4;
                    point.Z /= num4;
                }
            }
        }

        internal void MultiplyPoint(ref Point4D point)
        {
            if (!this.IsDistinguishedIdentity)
            {
                double x = point.X;
                double y = point.Y;
                double z = point.Z;
                double w = point.W;
                point.X = (((x * _m11) + (y * _m21)) + (z * _m31)) + (w * _offsetX);
                point.Y = (((x * _m12) + (y * _m22)) + (z * _m32)) + (w * _offsetY);
                point.Z = (((x * _m13) + (y * _m23)) + (z * _m33)) + (w * _offsetZ);
                point.W = (((x * _m14) + (y * _m24)) + (z * _m34)) + (w * _m44);
            }
        }

        internal void MultiplyVector(ref Vector3D vector)
        {
            if (!this.IsDistinguishedIdentity)
            {
                double x = vector.X;
                double y = vector.Y;
                double z = vector.Z;
                vector.X = ((x * _m11) + (y * _m21)) + (z * _m31);
                vector.Y = ((x * _m12) + (y * _m22)) + (z * _m32);
                vector.Z = ((x * _m13) + (y * _m23)) + (z * _m33);
            }
        }

        internal double GetNormalizedAffineDeterminant()
        {
            double num3 = (_m12 * _m23) - (_m22 * _m13);
            double num2 = (_m32 * _m13) - (_m12 * _m33);
            double num = (_m22 * _m33) - (_m32 * _m23);
            return (((_m31 * num3) + (_m21 * num2)) + (_m11 * num));
        }

        internal bool NormalizedAffineInvert()
        {
            double num11 = (_m12 * _m23) - (_m22 * _m13);
            double num10 = (_m32 * _m13) - (_m12 * _m33);
            double num9 = (_m22 * _m33) - (_m32 * _m23);
            double num8 = ((_m31 * num11) + (_m21 * num10)) + (_m11 * num9);
            if (IsZero(num8))
            {
                return false;
            }
            double num20 = (_m21 * _m13) - (_m11 * _m23);
            double num19 = (_m11 * _m33) - (_m31 * _m13);
            double num18 = (_m31 * _m23) - (_m21 * _m33);
            double num7 = (_m11 * _m22) - (_m21 * _m12);
            double num6 = (_m11 * _m32) - (_m31 * _m12);
            double num5 = (_m11 * _offsetY) - (_offsetX * _m12);
            double num4 = (_m21 * _m32) - (_m31 * _m22);
            double num3 = (_m21 * _offsetY) - (_offsetX * _m22);
            double num2 = (_m31 * _offsetY) - (_offsetX * _m32);
            double num17 = ((_m23 * num5) - (_offsetZ * num7)) - (_m13 * num3);
            double num16 = ((_m13 * num2) - (_m33 * num5)) + (_offsetZ * num6);
            double num15 = ((_m33 * num3) - (_offsetZ * num4)) - (_m23 * num2);
            double num14 = num7;
            double num13 = -num6;
            double num12 = num4;
            double num = 1.0 / num8;
            _m11 = num9 * num;
            _m12 = num10 * num;
            _m13 = num11 * num;
            _m21 = num18 * num;
            _m22 = num19 * num;
            _m23 = num20 * num;
            _m31 = num12 * num;
            _m32 = num13 * num;
            _m33 = num14 * num;
            _offsetX = num15 * num;
            _offsetY = num16 * num;
            _offsetZ = num17 * num;
            return true;
        }

        internal bool InvertCore()
        {
            if (!this.IsDistinguishedIdentity)
            {
                if (this.IsAffine)
                {
                    return this.NormalizedAffineInvert();
                }
                double num7 = (_m13 * _m24) - (_m23 * _m14);
                double num6 = (_m13 * _m34) - (_m33 * _m14);
                double num5 = (_m13 * _m44) - (_offsetZ * _m14);
                double num4 = (_m23 * _m34) - (_m33 * _m24);
                double num3 = (_m23 * _m44) - (_offsetZ * _m24);
                double num2 = (_m33 * _m44) - (_offsetZ * _m34);
                double num12 = ((_m22 * num6) - (_m32 * num7)) - (_m12 * num4);
                double num11 = ((_m12 * num3) - (_m22 * num5)) + (_offsetY * num7);
                double num10 = ((_m32 * num5) - (_offsetY * num6)) - (_m12 * num2);
                double num9 = ((_m22 * num2) - (_m32 * num3)) + (_offsetY * num4);
                double num8 = (((_offsetX * num12) + (_m31 * num11)) + (_m21 * num10)) + (_m11 * num9);
                if (IsZero(num8))
                {
                    return false;
                }
                double num24 = ((_m11 * num4) - (_m21 * num6)) + (_m31 * num7);
                double num23 = ((_m21 * num5) - (_offsetX * num7)) - (_m11 * num3);
                double num22 = ((_m11 * num2) - (_m31 * num5)) + (_offsetX * num6);
                double num21 = ((_m31 * num3) - (_offsetX * num4)) - (_m21 * num2);
                num7 = (_m11 * _m22) - (_m21 * _m12);
                num6 = (_m11 * _m32) - (_m31 * _m12);
                num5 = (_m11 * _offsetY) - (_offsetX * _m12);
                num4 = (_m21 * _m32) - (_m31 * _m22);
                num3 = (_m21 * _offsetY) - (_offsetX * _m22);
                num2 = (_m31 * _offsetY) - (_offsetX * _m32);
                double num20 = ((_m13 * num4) - (_m23 * num6)) + (_m33 * num7);
                double num19 = ((_m23 * num5) - (_offsetZ * num7)) - (_m13 * num3);
                double num18 = ((_m13 * num2) - (_m33 * num5)) + (_offsetZ * num6);
                double num17 = ((_m33 * num3) - (_offsetZ * num4)) - (_m23 * num2);
                double num16 = ((_m24 * num6) - (_m34 * num7)) - (_m14 * num4);
                double num15 = ((_m14 * num3) - (_m24 * num5)) + (_m44 * num7);
                double num14 = ((_m34 * num5) - (_m44 * num6)) - (_m14 * num2);
                double num13 = ((_m24 * num2) - (_m34 * num3)) + (_m44 * num4);
                double num = 1.0 / num8;
                _m11 = num9 * num;
                _m12 = num10 * num;
                _m13 = num11 * num;
                _m14 = num12 * num;
                _m21 = num21 * num;
                _m22 = num22 * num;
                _m23 = num23 * num;
                _m24 = num24 * num;
                _m31 = num13 * num;
                _m32 = num14 * num;
                _m33 = num15 * num;
                _m34 = num16 * num;
                _offsetX = num17 * num;
                _offsetY = num18 * num;
                _offsetZ = num19 * num;
                _m44 = num20 * num;
            }
            return true;
        }

        private static Matrix3D CreateIdentity() =>
            new Matrix3D(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0) { IsDistinguishedIdentity = true };

        private bool IsDistinguishedIdentity
        {
            get
            {
                return !_isNotKnownToBeIdentity;
            }
            set
            {
                _isNotKnownToBeIdentity = !value;
            }
        }
        public static bool operator ==(Matrix3D matrix1, Matrix3D matrix2)
        {
            if (matrix1.IsDistinguishedIdentity || matrix2.IsDistinguishedIdentity)
            {
                return (matrix1.IsIdentity == matrix2.IsIdentity);
            }
            return ((((((matrix1.M11 == matrix2.M11) && (matrix1.M12 == matrix2.M12)) && ((matrix1.M13 == matrix2.M13) && (matrix1.M14 == matrix2.M14))) && (((matrix1.M21 == matrix2.M21) && (matrix1.M22 == matrix2.M22)) && ((matrix1.M23 == matrix2.M23) && (matrix1.M24 == matrix2.M24)))) && ((((matrix1.M31 == matrix2.M31) && (matrix1.M32 == matrix2.M32)) && ((matrix1.M33 == matrix2.M33) && (matrix1.M34 == matrix2.M34))) && (((matrix1.OffsetX == matrix2.OffsetX) && (matrix1.OffsetY == matrix2.OffsetY)) && (matrix1.OffsetZ == matrix2.OffsetZ)))) && (matrix1.M44 == matrix2.M44));
        }

        public static bool operator !=(Matrix3D matrix1, Matrix3D matrix2) =>
            !(matrix1 == matrix2);

        public static bool Equals(Matrix3D matrix1, Matrix3D matrix2)
        {
            if (matrix1.IsDistinguishedIdentity || matrix2.IsDistinguishedIdentity)
            {
                return (matrix1.IsIdentity == matrix2.IsIdentity);
            }
            return (((((matrix1.M11.Equals(matrix2.M11) && matrix1.M12.Equals(matrix2.M12)) && (matrix1.M13.Equals(matrix2.M13) && matrix1.M14.Equals(matrix2.M14))) && ((matrix1.M21.Equals(matrix2.M21) && matrix1.M22.Equals(matrix2.M22)) && (matrix1.M23.Equals(matrix2.M23) && matrix1.M24.Equals(matrix2.M24)))) && (((matrix1.M31.Equals(matrix2.M31) && matrix1.M32.Equals(matrix2.M32)) && (matrix1.M33.Equals(matrix2.M33) && matrix1.M34.Equals(matrix2.M34))) && ((matrix1.OffsetX.Equals(matrix2.OffsetX) && matrix1.OffsetY.Equals(matrix2.OffsetY)) && matrix1.OffsetZ.Equals(matrix2.OffsetZ)))) && matrix1.M44.Equals(matrix2.M44));
        }

        public override bool Equals(object o)
        {
            if ((o == null) || !(o is Matrix3D))
            {
                return false;
            }
            Matrix3D matrixd = (Matrix3D)o;
            return Equals(this, matrixd);
        }

        public bool Equals(Matrix3D value) =>
            Equals(this, value);

        public override int GetHashCode()
        {
            if (this.IsDistinguishedIdentity)
            {
                return 0;
            }
            return (((((((((((((((this.M11.GetHashCode() ^ this.M12.GetHashCode()) ^ this.M13.GetHashCode()) ^ this.M14.GetHashCode()) ^ this.M21.GetHashCode()) ^ this.M22.GetHashCode()) ^ this.M23.GetHashCode()) ^ this.M24.GetHashCode()) ^ this.M31.GetHashCode()) ^ this.M32.GetHashCode()) ^ this.M33.GetHashCode()) ^ this.M34.GetHashCode()) ^ this.OffsetX.GetHashCode()) ^ this.OffsetY.GetHashCode()) ^ this.OffsetZ.GetHashCode()) ^ this.M44.GetHashCode());
        }

        public override string ToString() =>
            this.ConvertToString(null, null);

        public string ToString(IFormatProvider provider) =>
            this.ConvertToString(null, provider);

        string IFormattable.ToString(string format, IFormatProvider provider) =>
            this.ConvertToString(format, provider);

        internal string ConvertToString(string format, IFormatProvider provider)
        {
            if (this.IsIdentity)
            {
                return "Identity";
            }
            char numericListSeparator = TokenizerHelper.GetNumericListSeparator(provider);
            string[] textArray1 = new string[] {
                "{1:", format, "}{0}{2:", format, "}{0}{3:", format, "}{0}{4:", format, "}{0}{5:", format, "}{0}{6:", format, "}{0}{7:", format, "}{0}{8:", format,
                "}{0}{9:", format, "}{0}{10:", format, "}{0}{11:", format, "}{0}{12:", format, "}{0}{13:", format, "}{0}{14:", format, "}{0}{15:", format, "}{0}{16:", format,
                "}"
            };
            object[] args = new object[] {
                numericListSeparator, _m11, _m12, _m13, _m14, _m21, _m22, _m23, _m24, _m31, _m32, _m33, _m34, _offsetX, _offsetY, _offsetZ,
                _m44
            };
            return string.Format(provider, string.Concat(textArray1), args);
        }

        static Matrix3D()
        {
            s_identity = CreateIdentity();
        }

        public static bool IsZero(double value) =>
            (Math.Abs(value) < 2.2204460492503131E-15);
    }
}
