#ifndef FORMAT_H
#define FORMAT_H

#include <QString>
#include <cmath>
#include <cstdlib>
#include <limits>

namespace Fmt{
    inline QString groupThousands(const QString &digits){
        const int n = digits.length();
        QString out;
        out.reserve(n + n / 3);
        for (int i = 0; i < n; ++i){
            out += digits.at(i);
            const int remaining = n - (i + 1);
            if (remaining >= 3 && (remaining % 3) == 0)
                out += QLatin1Char('.');
        }
        return out;
    }

    inline QString toFixed2(double value){
        return QString::number(value, 'f', 2);
    }

    inline QString formatNumber(double num){
        const QString fixed = toFixed2(num);
        const int dot = fixed.indexOf(QLatin1Char('.'));
        const QString intPart = dot >= 0 ? fixed.left(dot) : fixed;
        const QString decPart = dot >= 0 ? fixed.mid(dot + 1) : QString();
        return groupThousands(intPart) + QLatin1Char(',') + decPart;
    }

    struct BalanceParts{
        QString intPart;
        QString decPart;
        bool negative = false;
    };
    
    inline BalanceParts balanceParts(double amount){
        BalanceParts p;
        p.negative = amount < 0;
        const QString fixed = toFixed2(std::abs(amount));
        const int dot = fixed.indexOf(QLatin1Char('.'));
        p.intPart = dot >= 0 ? fixed.left(dot) : fixed;
        p.decPart = dot >= 0 ? fixed.mid(dot + 1) : QString();
        return p;
    }

    inline double parseJsFloat(const QString &raw, bool &ok){
        ok = false;
        QString s;
        for (const QChar ch : raw){
            if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' || ch == '\f' || ch == '\v')
                continue;
            s += ch;
        }
        const int comma = s.indexOf(QLatin1Char(','));
        if (comma >= 0)
            s = s.left(comma) + QChar('.') + s.mid(comma + 1);
        s = s.trimmed();

        int i = 0;
        const int n = s.size();
        bool negative = false;
        if (i < n && (s.at(i) == QLatin1Char('+') || s.at(i) == QLatin1Char('-'))){
            negative = s.at(i) == QLatin1Char('-');
            ++i;
        }

        long double mantissa = 0.0L;
        int exponent = 0;
        int digits = 0;
        bool sawDot = false;

        while (i < n){
            const QChar c = s.at(i);
            if (c == QLatin1Char('.')){
                if (sawDot)
                    break;
                sawDot = true;
                ++i;
                continue;
            }
            const ushort u = c.unicode();
            if (u < '0' || u > '9')
                break;
            mantissa = mantissa * 10.0L + (u - '0');
            if (sawDot)
                --exponent;
            ++digits;
            ++i;
        }

        if (digits == 0)
            return std::numeric_limits<double>::quiet_NaN();

        if (i < n && (s.at(i) == QLatin1Char('e') || s.at(i) == QLatin1Char('E'))){
            int j = i + 1;
            bool expNegative = false;
            if (j < n && (s.at(j) == QLatin1Char('+') || s.at(j) == QLatin1Char('-'))){
                expNegative = s.at(j) == QLatin1Char('-');
                ++j;
            }
            
            int expValue = 0;
            int expDigits = 0;
            while (j < n){
                const ushort u = s.at(j).unicode();
                if (u < '0' || u > '9')
                    break;
                if (expValue < 100000)
                    expValue = expValue * 10 + (u - '0');
                ++expDigits;
                ++j;
            }
            if (expDigits > 0){
                exponent += expNegative ? -expValue : expValue;
                i = j;
            }
        }

        if (digits == 0)
            return std::numeric_limits<double>::quiet_NaN();

        const long double value = mantissa * std::pow(10.0L, exponent);
        ok = true;
        return negative ? double(-value) : double(value);
    }

    inline QString formatTrLocale(double parsed){
        long long units = std::floor(std::abs(parsed) + 1e-9);
        double diff = parsed - std::floor(parsed);
        if (std::abs(diff) < 1e-9){
            QString digits = QString::number(units);
            return groupThousands(digits);
        }
        QString v = QString::number(parsed, 'f', 3);
        const int dot = v.indexOf(QLatin1Char('.'));
        QString intPart = v.left(dot);
        QString frac = v.mid(dot + 1);
        while (frac.length() > 2 && frac.endsWith(QLatin1Char('0')))
            frac.chop(1);
        if (intPart.isEmpty())
            intPart = QStringLiteral("0");
        const bool negative = parsed < 0;
        QString grouped = groupThousands(negative ? intPart.mid(1) : intPart);
        return (negative ? QStringLiteral("-") : QString()) + grouped + QLatin1Char(',') + frac;
    }
}

#endif
