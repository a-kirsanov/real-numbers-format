#include <iostream>
#include <bitset>
#include <iomanip>
#include <cmath>
#include <sstream>

using namespace std;

template <typename T>
struct RealFormat_traits {};

template <>
struct RealFormat_traits<float> {
    static constexpr auto mantis_length = 8;
    using unsigned_type = unsigned int;
};

template <>
struct RealFormat_traits<double> {
    static constexpr auto mantis_length = 11;
    using unsigned_type = unsigned long int;
};

template <typename T>
union RealUnion {
    using traits = RealFormat_traits<T>;
    T value;
    typename traits::unsigned_type u;

    static_assert(sizeof(T) == sizeof(typename traits::unsigned_type), "wrong union");

    string to_string(bool sep = true) const {
        const auto s_repr = std::bitset<sizeof(T) * 8>(u).to_string();
        return s_repr.substr(0, 1) + (sep ? " " : "") + s_repr.substr(1, traits::mantis_length) + (sep ? " " : "") +
               s_repr.substr(traits::mantis_length + 1, s_repr.size());
    }

    RealUnion(T value) : value(value) {}
};

template <typename T>
struct RealFormat {
    RealFormat(T value) : un(value) {}

    /** Memory bits layout */
    string mem() const {
        return un.to_string();
    }

    /** Scientific format */
    string sc() const {
        const auto s = un.to_string(false);
        const std::string sign = s[0] == 0x31 ? "-" : "+";
        const auto expo_s = s.substr(1, RealUnion<T>::traits::mantis_length);
        const auto expo = std::stoi(expo_s, 0, 2) - 1023;
        const std::string fractions_s = s.substr(RealUnion<T>::traits::mantis_length + 1, s.size());

        return sign + (expo == -1023 ? "0." : "1.") + frac_to_string(fractions_s) + " e" +
               to_string(expo == -1023 ? -1022 : expo);
    }

    static string frac_to_string(const string &frac) {
        double frac_value = 0.;
        int i = -1;
        for (auto &f : frac) {
            if (f == 0x31) {
                frac_value += std::pow(2., i);
            }
            --i;
        }

        std::ostringstream oss;
        oss << std::setprecision(16) << std::fixed << frac_value;

        return oss.str().substr(2, 100);
    }

 private:
    RealUnion<T> un;
};

int main() {
    using insp_type = double;

    for (auto &v : {1., 1.5, 1.7}) {
        const RealFormat<insp_type> format{static_cast<insp_type>(v)};
        std::cout << v << "\t" << format.mem() << "\t" << format.sc() << std::endl;
    }

    insp_type d = 1.;

    std::cout << std::setprecision(40) << std::fixed;
    while (d != 0.) {
        const RealFormat<insp_type> format{d};
        std::cout << d << "\t" << format.mem() << "\t" << format.sc() << std::endl;
        d /= 2.;
    }
}
