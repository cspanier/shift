using System;
using System.Globalization;

namespace Thales.Sagittarius.Data.Media3D
{
    internal class TokenizerHelper
    {
        private char _argSeparator;
        private int _charIndex;
        internal int _currentTokenIndex;
        internal int _currentTokenLength;
        private bool _foundSeparator;
        private char _quoteChar;
        private string _str;
        private int _strLen;

        internal TokenizerHelper(string str, IFormatProvider formatProvider)
        {
            char numericListSeparator = GetNumericListSeparator(formatProvider);
            this.Initialize(str, '\'', numericListSeparator);
        }

        internal TokenizerHelper(string str, char quoteChar, char separator)
        {
            this.Initialize(str, quoteChar, separator);
        }

        internal string GetCurrentToken()
        {
            if (this._currentTokenIndex < 0)
            {
                return null;
            }
            return _str.Substring(this._currentTokenIndex, this._currentTokenLength);
        }

        internal static char GetNumericListSeparator(IFormatProvider provider)
        {
            char ch = ',';
            NumberFormatInfo instance = NumberFormatInfo.GetInstance(provider);
            if ((instance.NumberDecimalSeparator.Length > 0) && (ch == instance.NumberDecimalSeparator[0]))
            {
                ch = ';';
            }
            return ch;
        }

        private void Initialize(string str, char quoteChar, char separator)
        {
            _str = str;
            _strLen = (str == null) ? 0 : str.Length;
            this._currentTokenIndex = -1;
            _quoteChar = quoteChar;
            _argSeparator = separator;
            while (_charIndex < _strLen)
            {
                if (!char.IsWhiteSpace(_str, _charIndex))
                {
                    break;
                }
                _charIndex++;
            }
        }

        internal void LastTokenRequired()
        {
            if (_charIndex != _strLen)
            {
                object[] args = new object[] { _charIndex, _str };
                throw new InvalidOperationException("TokenizerHelperExtraDataEncountered");
            }
        }

        internal bool NextToken() =>
            this.NextToken(false);

        internal bool NextToken(bool allowQuotedToken) =>
            this.NextToken(allowQuotedToken, _argSeparator);

        internal bool NextToken(bool allowQuotedToken, char separator)
        {
            this._currentTokenIndex = -1;
            _foundSeparator = false;
            if (_charIndex >= _strLen)
            {
                return false;
            }
            char c = _str[_charIndex];
            int num = 0;
            if (allowQuotedToken && (c == _quoteChar))
            {
                num++;
                _charIndex++;
            }
            int num2 = _charIndex;
            int num3 = 0;
            while (_charIndex < _strLen)
            {
                c = _str[_charIndex];
                if (num > 0)
                {
                    if (c != _quoteChar)
                    {
                        goto Label_00AA;
                    }
                    num--;
                    if (num != 0)
                    {
                        goto Label_00AA;
                    }
                    _charIndex++;
                    break;
                }
                if (char.IsWhiteSpace(c) || (c == separator))
                {
                    if (c == separator)
                    {
                        _foundSeparator = true;
                    }
                    break;
                }
                Label_00AA:
                _charIndex++;
                num3++;
            }
            if (num > 0)
            {
                object[] args = new object[] { _str };
                throw new InvalidOperationException("TokenizerHelperMissingEndQuote");
            }
            this.ScanToNextToken(separator);
            this._currentTokenIndex = num2;
            this._currentTokenLength = num3;
            if (this._currentTokenLength < 1)
            {
                object[] objArray2 = new object[] { _charIndex, _str };
                throw new InvalidOperationException("TokenizerHelperEmptyToken");
            }
            return true;
        }

        internal string NextTokenRequired()
        {
            if (!this.NextToken(false))
            {
                object[] args = new object[] { _str };
                throw new InvalidOperationException("TokenizerHelperPrematureStringTermination");
            }
            return this.GetCurrentToken();
        }

        internal string NextTokenRequired(bool allowQuotedToken)
        {
            if (!this.NextToken(allowQuotedToken))
            {
                object[] args = new object[] { _str };
                throw new InvalidOperationException("TokenizerHelperPrematureStringTermination");
            }
            return this.GetCurrentToken();
        }

        private void ScanToNextToken(char separator)
        {
            if (_charIndex < _strLen)
            {
                char c = _str[_charIndex];
                if ((c != separator) && !char.IsWhiteSpace(c))
                {
                    object[] args = new object[] { _charIndex, _str };
                    throw new InvalidOperationException("TokenizerHelperExtraDataEncountered");
                }
                int num = 0;
                while (_charIndex < _strLen)
                {
                    c = _str[_charIndex];
                    if (c == separator)
                    {
                        _foundSeparator = true;
                        num++;
                        _charIndex++;
                        if (num > 1)
                        {
                            object[] objArray2 = new object[] { _charIndex, _str };
                            throw new InvalidOperationException("TokenizerHelperEmptyToken");
                        }
                    }
                    else
                    {
                        if (!char.IsWhiteSpace(c))
                        {
                            break;
                        }
                        _charIndex++;
                    }
                }
                if ((num > 0) && (_charIndex >= _strLen))
                {
                    object[] objArray3 = new object[] { _charIndex, _str };
                    throw new InvalidOperationException("TokenizerHelperEmptyToken");
                }
            }
        }

        internal bool FoundSeparator =>
            _foundSeparator;
    }
}
