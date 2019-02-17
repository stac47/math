
///////////////////////////////////////////////////////////////////////////////
//  Copyright 2018 John Maddock
//  Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_MATH_HYPERGEOMETRIC_PFQ_HPP
#define BOOST_MATH_HYPERGEOMETRIC_PFQ_HPP

#include <boost/math/special_functions/detail/hypergeometric_pFq_checked_series.hpp>
#include <boost/chrono.hpp>

namespace boost {
   namespace math {

      namespace detail {

         struct pFq_termination_exception : public std::runtime_error
         {
            pFq_termination_exception(const char* p) : std::runtime_error(p) {}
         };

         struct timed_iteration_terminator
         {
            timed_iteration_terminator(boost::uintmax_t i, double t) : max_iter(i), max_time(t), start_time(boost::chrono::system_clock::now()) {}

            bool operator()(boost::uintmax_t iter)const
            {
               if (iter > max_iter)
                  boost::throw_exception(boost::math::detail::pFq_termination_exception("pFq exceeded maximum permitted iterations."));
               if (boost::chrono::duration<double>(boost::chrono::system_clock::now() - start_time).count() > max_time)
                  boost::throw_exception(boost::math::detail::pFq_termination_exception("pFq exceeded maximum permitted evaluation time."));
               return false;
            }

            boost::uintmax_t max_iter;
            double max_time;
            boost::chrono::system_clock::time_point start_time;
         };

      }

      template <class Seq, class Real, class Policy>
      inline Real hypergeometric_pFq(const Seq& aj, const Seq& bj, const Real& z, Real* pNorm, const Policy& pol)
      {
         int scale = 0;
         std::pair<Real, Real> r = boost::math::detail::hypergeometric_pFq_checked_series_impl(aj, bj, z, pol, boost::math::detail::iteration_terminator(boost::math::policies::get_max_series_iterations<Policy>()), scale);
         r.first *= exp(Real(scale));
         r.second *= exp(Real(scale));
         if (pNorm)
            *pNorm = r.second;
         return r.first;
      }

      template <class Seq, class Real>
      inline std::pair<Real, Real> hypergeometric_pFq(const Seq& aj, const Seq& bj, const Real& z, Real* pNorm = 0)
      {
         return hypergeometric_pFq(aj, bj, z, pNorm, boost::math::policies::policy<>());
      }

      template <class Seq, class Real, class Policy>
      Real hypergeometric_pFq_precision(const Seq& aj, const Seq& bj, const Real& z, unsigned digits10, double timeout, const Policy& pol)
      {
         unsigned current_precision = digits10 + 5;

         for (auto ai = aj.begin(); ai != aj.end(); ++ai)
         {
            current_precision = std::max(current_precision, ai->precision());
         }
         for (auto bi = bj.begin(); bi != bj.end(); ++bi)
         {
            current_precision = std::max(current_precision, bi->precision());
         }
         current_precision = std::max(current_precision, z.precision());

         Real r, norm;
         std::vector<Real> aa(aj), bb(bj);
         do
         {
            Real::default_precision(current_precision);
            for (auto ai = aa.begin(); ai != aa.end(); ++ai)
               ai->precision(current_precision);
            for (auto bi = bb.begin(); bi != bb.end(); ++bi)
               bi->precision(current_precision);
            try
            {
               int scale = 0;
               std::pair<Real, Real> rp = boost::math::detail::hypergeometric_pFq_checked_series_impl(aj, bj, z, pol, boost::math::detail::timed_iteration_terminator(boost::math::policies::get_max_series_iterations<Policy>(), timeout), scale);
               rp.first *= exp(Real(scale));
               rp.second *= exp(Real(scale));

               r = rp.first;
               norm = rp.second;

               unsigned precision_obtained = current_precision - 1 - itrunc(log10(abs(norm / r)));
               if (precision_obtained < digits10)
               {
                  current_precision += digits10 - precision_obtained + 5;
               }
               else
                  break;
            }
            catch (const boost::math::evaluation_error&)
            {
               current_precision *= 2;
            }
            catch (const detail::pFq_termination_exception& e)
            {
               //
               // Either we have exhausted the number of series iterations, or the timeout.
               // Either way we quit now.
               throw boost::math::evaluation_error(e.what());
            }
         } while (true);

         return r;
      }
      template <class Seq, class Real>
      Real hypergeometric_pFq_precision(const Seq& aj, const Seq& bj, const Real& z, unsigned digits10, double timeout = 0.5)
      {
         return hypergeometric_pFq_precision(aj, bj, z, digits10, timeout, boost::math::policies::policy<>());
      }

      template <class Real, class Policy>
      Real hypergeometric_pFq_precision(const std::initializer_list<Real>& aj, const std::initializer_list<Real>& bj, const Real& z, unsigned digits10, double timeout, const Policy& pol)
      {
         return hypergeometric_pFq_precision< std::initializer_list<Real>, Real>(aj, bj, z, digits10, timeout, pol);
      }
      template <class Real>
      Real hypergeometric_pFq_precision(const std::initializer_list<Real>& aj, const std::initializer_list<Real>& bj, const Real& z, unsigned digits10, double timeout = 0.5)
      {
         return hypergeometric_pFq_precision< std::initializer_list<Real>, Real>(aj, bj, z, digits10, timeout, boost::math::policies::policy<>());
      }

   }
} // namespaces

#endif // BOOST_MATH_BESSEL_ITERATORS_HPP
