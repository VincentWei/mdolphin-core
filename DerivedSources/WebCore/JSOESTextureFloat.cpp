/*
    This file is part of the WebKit open source project.
    This file has been generated by generate-bindings.pl. DO NOT MODIFY!

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "config.h"

#if ENABLE(WEBGL)

#include "JSOESTextureFloat.h"

#include "OESTextureFloat.h"
#include <wtf/GetPtr.h>

using namespace JSC;

namespace WebCore {

ASSERT_CLASS_FITS_IN_CELL(JSOESTextureFloat);

/* Hash table for prototype */
#if ENABLE(JIT)
#define THUNK_GENERATOR(generator) , generator
#else
#define THUNK_GENERATOR(generator)
#endif

static const HashTableValue JSOESTextureFloatPrototypeTableValues[1] =
{
    { 0, 0, 0, 0 THUNK_GENERATOR(0) }
};

#undef THUNK_GENERATOR
static JSC_CONST_HASHTABLE HashTable JSOESTextureFloatPrototypeTable = { 1, 0, JSOESTextureFloatPrototypeTableValues, 0 };
const ClassInfo JSOESTextureFloatPrototype::s_info = { "OESTextureFloatPrototype", &JSC::JSObjectWithGlobalObject::s_info, &JSOESTextureFloatPrototypeTable, 0 };

JSObject* JSOESTextureFloatPrototype::self(ExecState* exec, JSGlobalObject* globalObject)
{
    return getDOMPrototype<JSOESTextureFloat>(exec, globalObject);
}

const ClassInfo JSOESTextureFloat::s_info = { "OESTextureFloat", &DOMObjectWithGlobalPointer::s_info, 0, 0 };

JSOESTextureFloat::JSOESTextureFloat(NonNullPassRefPtr<Structure> structure, JSDOMGlobalObject* globalObject, PassRefPtr<OESTextureFloat> impl)
    : DOMObjectWithGlobalPointer(structure, globalObject)
    , m_impl(impl)
{
    ASSERT(inherits(&s_info));
}

JSObject* JSOESTextureFloat::createPrototype(ExecState* exec, JSGlobalObject* globalObject)
{
    return new (exec) JSOESTextureFloatPrototype(globalObject, JSOESTextureFloatPrototype::createStructure(globalObject->globalData(), globalObject->objectPrototype()));
}

JSC::JSValue toJS(JSC::ExecState* exec, JSDOMGlobalObject* globalObject, OESTextureFloat* object)
{
    return getDOMObjectWrapper<JSOESTextureFloat>(exec, globalObject, object);
}
OESTextureFloat* toOESTextureFloat(JSC::JSValue value)
{
    return value.inherits(&JSOESTextureFloat::s_info) ? static_cast<JSOESTextureFloat*>(asObject(value))->impl() : 0;
}

}

#endif // ENABLE(WEBGL)