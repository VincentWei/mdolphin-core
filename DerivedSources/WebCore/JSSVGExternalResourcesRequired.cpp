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

#if ENABLE(SVG)

#include "JSSVGExternalResourcesRequired.h"

#include "JSSVGAnimatedBoolean.h"
#include "SVGExternalResourcesRequired.h"
#include <wtf/GetPtr.h>

using namespace JSC;

namespace WebCore {

ASSERT_CLASS_FITS_IN_CELL(JSSVGExternalResourcesRequired);

/* Hash table */
#if ENABLE(JIT)
#define THUNK_GENERATOR(generator) , generator
#else
#define THUNK_GENERATOR(generator)
#endif

static const HashTableValue JSSVGExternalResourcesRequiredTableValues[2] =
{
    { "externalResourcesRequired", DontDelete | ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGExternalResourcesRequiredExternalResourcesRequired), (intptr_t)0 THUNK_GENERATOR(0) },
    { 0, 0, 0, 0 THUNK_GENERATOR(0) }
};

#undef THUNK_GENERATOR
static JSC_CONST_HASHTABLE HashTable JSSVGExternalResourcesRequiredTable = { 2, 1, JSSVGExternalResourcesRequiredTableValues, 0 };
/* Hash table for prototype */
#if ENABLE(JIT)
#define THUNK_GENERATOR(generator) , generator
#else
#define THUNK_GENERATOR(generator)
#endif

static const HashTableValue JSSVGExternalResourcesRequiredPrototypeTableValues[1] =
{
    { 0, 0, 0, 0 THUNK_GENERATOR(0) }
};

#undef THUNK_GENERATOR
static JSC_CONST_HASHTABLE HashTable JSSVGExternalResourcesRequiredPrototypeTable = { 1, 0, JSSVGExternalResourcesRequiredPrototypeTableValues, 0 };
const ClassInfo JSSVGExternalResourcesRequiredPrototype::s_info = { "SVGExternalResourcesRequiredPrototype", &JSC::JSObjectWithGlobalObject::s_info, &JSSVGExternalResourcesRequiredPrototypeTable, 0 };

JSObject* JSSVGExternalResourcesRequiredPrototype::self(ExecState* exec, JSGlobalObject* globalObject)
{
    return getDOMPrototype<JSSVGExternalResourcesRequired>(exec, globalObject);
}

const ClassInfo JSSVGExternalResourcesRequired::s_info = { "SVGExternalResourcesRequired", &DOMObjectWithGlobalPointer::s_info, &JSSVGExternalResourcesRequiredTable, 0 };

JSSVGExternalResourcesRequired::JSSVGExternalResourcesRequired(NonNullPassRefPtr<Structure> structure, JSDOMGlobalObject* globalObject, PassRefPtr<SVGExternalResourcesRequired> impl)
    : DOMObjectWithGlobalPointer(structure, globalObject)
    , m_impl(impl)
{
    ASSERT(inherits(&s_info));
}

JSObject* JSSVGExternalResourcesRequired::createPrototype(ExecState* exec, JSGlobalObject* globalObject)
{
    return new (exec) JSSVGExternalResourcesRequiredPrototype(globalObject, JSSVGExternalResourcesRequiredPrototype::createStructure(globalObject->globalData(), globalObject->objectPrototype()));
}

bool JSSVGExternalResourcesRequired::getOwnPropertySlot(ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
{
    return getStaticValueSlot<JSSVGExternalResourcesRequired, Base>(exec, &JSSVGExternalResourcesRequiredTable, this, propertyName, slot);
}

bool JSSVGExternalResourcesRequired::getOwnPropertyDescriptor(ExecState* exec, const Identifier& propertyName, PropertyDescriptor& descriptor)
{
    return getStaticValueDescriptor<JSSVGExternalResourcesRequired, Base>(exec, &JSSVGExternalResourcesRequiredTable, this, propertyName, descriptor);
}

JSValue jsSVGExternalResourcesRequiredExternalResourcesRequired(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSSVGExternalResourcesRequired* castedThis = static_cast<JSSVGExternalResourcesRequired*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    SVGExternalResourcesRequired* imp = static_cast<SVGExternalResourcesRequired*>(castedThis->impl());
    RefPtr<SVGAnimatedBoolean> obj = imp->externalResourcesRequiredAnimated();
    JSValue result =  toJS(exec, castedThis->globalObject(), obj.get());
    return result;
}

JSC::JSValue toJS(JSC::ExecState* exec, JSDOMGlobalObject* globalObject, SVGExternalResourcesRequired* object)
{
    return getDOMObjectWrapper<JSSVGExternalResourcesRequired>(exec, globalObject, object);
}
SVGExternalResourcesRequired* toSVGExternalResourcesRequired(JSC::JSValue value)
{
    return value.inherits(&JSSVGExternalResourcesRequired::s_info) ? static_cast<JSSVGExternalResourcesRequired*>(asObject(value))->impl() : 0;
}

}

#endif // ENABLE(SVG)
