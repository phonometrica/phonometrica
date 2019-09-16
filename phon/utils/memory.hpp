/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                            *
 *                                                                                                                     *
 * This software is governed by the CeCILL license under French law and abiding by the rules of distribution of free   *
 * software. You can use, modify and/or redistribute the software under the terms of the CeCILL license as circulated  *
 * by CEA, CNRS and INRIA at the following URL "http://www.cecill.info".                                               *
 *                                                                                                                     *
 * As a counterpart to the access to the source code and rights to copy, modify and redistribute granted by the        *
 * license, users are provided only with a limited warranty and the software's author, the holder of the economic      *
 * rights, and the successive licensors have only limited liability.                                                   *
 *                                                                                                                     *
 * In this respect, the user's attention is drawn to the risks associated with loading, using, modifying and/or        *
 * developing or reproducing the software by the user in light of its specific status of free software, that may mean  *
 * that it is complicated to manipulate, and that also therefore means that it is reserved for developers and          *
 * experienced professionals having in-depth computer knowledge. Users are therefore encouraged to load and test the   *
 * software's suitability as regards their requirements in conditions enabling the security of their systems and/or    *
 * data to be ensured and, more generally, to use and operate it in the same conditions as regards security.           *
 *                                                                                                                     *
 * The fact that you are presently reading this means that you have had knowledge of the CeCILL license and that you   *
 * accept its terms.                                                                                                   *
 *                                                                                                                     *
 * Created: 20/02/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: convenience templates to deal with smart pointers.                                                         *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_MEMORY_HPP
#define PHONOMETRICA_MEMORY_HPP

#include <memory>

namespace phonometrica {

// Get a downcasted raw pointer from a shared pointer.
template<class Derived, class Base>
Derived *raw_cast(const std::shared_ptr<Base> &ptr)
{
	static_assert(std::is_base_of<std::remove_cv_t<Base> , std::remove_cv_t<Derived>>::value, "Base must be a base of Derived");
	return static_cast<Derived*>(ptr.get());
};

// Get a downcasted shared pointer from another shared pointer.
template<class Derived, class Base>
std::shared_ptr<Derived> downcast(const std::shared_ptr<Base> &ptr)
{
	static_assert(std::is_base_of<Base,Derived>::value, "Base must be a base of Derived");
	return std::static_pointer_cast<Derived>(ptr);
};

template<class Base, class Derived>
std::shared_ptr<Base> upcast(const std::shared_ptr<Derived> &ptr)
{
	static_assert(std::is_base_of<Base,Derived>::value, "Base must be a base of Derived");
	return std::static_pointer_cast<Base>(ptr);
};


} // namespace phonometrica

#endif // PHONOMETRICA_MEMORY_HPP
