/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 GPL Source Code (?Doom 3 Source Code?).

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "../Game_local.h"

// simple types.  function types are dynamically allocated
idTypeDef	type_void(ev_void, &def_void, "void", 0, NULL);
idTypeDef	type_scriptevent(ev_scriptevent, &def_scriptevent, "scriptevent", sizeof(intptr_t), NULL);
idTypeDef	type_namespace(ev_namespace, &def_namespace, "namespace", sizeof(intptr_t), NULL);
idTypeDef	type_string(ev_string, &def_string, "string", MAX_STRING_LEN, NULL);
idTypeDef	type_float(ev_float, &def_float, "float", sizeof(intptr_t), NULL);
idTypeDef	type_vector(ev_vector, &def_vector, "vector", E_EVENT_SIZEOF_VEC, NULL);
idTypeDef	type_entity(ev_entity, &def_entity, "entity", sizeof(intptr_t), NULL);					// stored as entity number pointer
idTypeDef	type_field(ev_field, &def_field, "field", sizeof(intptr_t), NULL);
idTypeDef	type_function(ev_function, &def_function, "function", sizeof(intptr_t), &type_void);
idTypeDef	type_virtualfunction(ev_virtualfunction, &def_virtualfunction, "virtual function", sizeof(intptr_t), NULL);
idTypeDef	type_pointer(ev_pointer, &def_pointer, "pointer", sizeof(intptr_t), NULL);
idTypeDef	type_object(ev_object, &def_object, "object", sizeof(intptr_t), NULL);					// stored as entity number pointer
idTypeDef	type_jumpoffset(ev_jumpoffset, &def_jumpoffset, "<jump>", sizeof(intptr_t), NULL);		// only used for jump opcodes
idTypeDef	type_argsize(ev_argsize, &def_argsize, "<argsize>", sizeof(intptr_t), NULL);				// only used for function call and thread opcodes
idTypeDef	type_boolean(ev_boolean, &def_boolean, "boolean", sizeof(intptr_t), NULL);

idVarDef	def_void(&type_void);
idVarDef	def_scriptevent(&type_scriptevent);
idVarDef	def_namespace(&type_namespace);
idVarDef	def_string(&type_string);
idVarDef	def_float(&type_float);
idVarDef	def_vector(&type_vector);
idVarDef	def_entity(&type_entity);
idVarDef	def_field(&type_field);
idVarDef	def_function(&type_function);
idVarDef	def_virtualfunction(&type_virtualfunction);
idVarDef	def_pointer(&type_pointer);
idVarDef	def_object(&type_object);
idVarDef	def_jumpoffset(&type_jumpoffset);		// only used for jump opcodes
idVarDef	def_argsize(&type_argsize);
idVarDef	def_boolean(&type_boolean);

/***********************************************************************

  function_t

***********************************************************************/

/*
================
function_t::function_t
================
*/
function_t::function_t()
{
	Clear();
}

/*
================
function_t::Allocated
================
*/
size_t function_t::Allocated(void) const
{
	return name.Allocated() + parmSize.Allocated();
}

/*
================
function_t::SetName
================
*/
void function_t::SetName(const char *name)
{
	this->name = name;
}

/*
================
function_t::Name
================
*/
const char *function_t::Name(void) const
{
	return name;
}

/*
================
function_t::Clear
================
*/
void function_t::Clear(void)
{
	eventdef		= NULL;
	def				= NULL;
	type			= NULL;
	firstStatement	= 0;
	numStatements	= 0;
	parmTotal		= 0;
	locals			= 0;
	filenum			= 0;
	name.Clear();
	parmSize.Clear();
}

/***********************************************************************

  idTypeDef

***********************************************************************/

/*
================
idTypeDef::idTypeDef
================
*/
idTypeDef::idTypeDef(etype_t etype, idVarDef *edef, const char *ename, int esize, idTypeDef *aux)
{
	name		= ename;
	type		= etype;
	def			= edef;
	size		= esize;
	auxType		= aux;

	parmTypes.SetGranularity(1);
	parmNames.SetGranularity(1);
	functions.SetGranularity(1);
}

/*
================
idTypeDef::idTypeDef
================
*/
idTypeDef::idTypeDef(const idTypeDef &other)
{
	*this = other;
}

/*
================
idTypeDef::operator=
================
*/
void idTypeDef::operator=(const idTypeDef &other)
{
	type		= other.type;
	def			= other.def;
	name		= other.name;
	size		= other.size;
	auxType		= other.auxType;
	parmTypes	= other.parmTypes;
	parmNames	= other.parmNames;
	functions	= other.functions;
}

/*
================
idTypeDef::Allocated
================
*/
size_t idTypeDef::Allocated(void) const
{
	size_t memsize;
	int i;

	memsize = name.Allocated() + parmTypes.Allocated() + parmNames.Allocated() + functions.Allocated();

	for (i = 0; i < parmTypes.Num(); i++) {
		memsize += parmNames[ i ].Allocated();
	}

	return memsize;
}

/*
================
idTypeDef::Inherits

Returns true if basetype is an ancestor of this type.
================
*/
bool idTypeDef::Inherits(const idTypeDef *basetype) const
{
	idTypeDef *superType;

	if (type != ev_object) {
		return false;
	}

	if (this == basetype) {
		return true;
	}

	for (superType = auxType; superType != NULL; superType = superType->auxType) {
		if (superType == basetype) {
			return true;
		}
	}

	return false;
}

/*
================
idTypeDef::MatchesType

Returns true if both types' base types and parameters match
================
*/
bool idTypeDef::MatchesType(const idTypeDef &matchtype) const
{
	int i;

	if (this == &matchtype) {
		return true;
	}

	if ((type != matchtype.type) || (auxType != matchtype.auxType)) {
		return false;
	}

	if (parmTypes.Num() != matchtype.parmTypes.Num()) {
		return false;
	}

	for (i = 0; i < matchtype.parmTypes.Num(); i++) {
		if (parmTypes[ i ] != matchtype.parmTypes[ i ]) {
			return false;
		}
	}

	return true;
}

/*
================
idTypeDef::MatchesVirtualFunction

Returns true if both functions' base types and parameters match
================
*/
bool idTypeDef::MatchesVirtualFunction(const idTypeDef &matchfunc) const
{
	int i;

	if (this == &matchfunc) {
		return true;
	}

	if ((type != matchfunc.type) || (auxType != matchfunc.auxType)) {
		return false;
	}

	if (parmTypes.Num() != matchfunc.parmTypes.Num()) {
		return false;
	}

	if (parmTypes.Num() > 0) {
		if (!parmTypes[ 0 ]->Inherits(matchfunc.parmTypes[ 0 ])) {
			return false;
		}
	}

	for (i = 1; i < matchfunc.parmTypes.Num(); i++) {
		if (parmTypes[ i ] != matchfunc.parmTypes[ i ]) {
			return false;
		}
	}

	return true;
}

/*
================
idTypeDef::AddFunctionParm

Adds a new parameter for a function type.
================
*/
void idTypeDef::AddFunctionParm(idTypeDef *parmtype, const char *name)
{
	if (type != ev_function) {
		throw idCompileError("idTypeDef::AddFunctionParm : tried to add parameter on non-function type");
	}

	parmTypes.Append(parmtype);
	idStr &parmName = parmNames.Alloc();
	parmName = name;
}

/*
================
idTypeDef::AddField

Adds a new field to an object type.
================
*/
void idTypeDef::AddField(idTypeDef *fieldtype, const char *name)
{
	if (type != ev_object) {
		throw idCompileError("idTypeDef::AddField : tried to add field to non-object type");
	}

	parmTypes.Append(fieldtype);
	idStr &parmName = parmNames.Alloc();
	parmName = name;

	if (fieldtype->FieldType()->Inherits(&type_object)) {
		size += type_object.Size();
	} else {
		size += fieldtype->FieldType()->Size();
	}
}

/*
================
idTypeDef::SetName
================
*/
void idTypeDef::SetName(const char *newname)
{
	name = newname;
}

/*
================
idTypeDef::Name
================
*/
const char *idTypeDef::Name(void) const
{
	return name;
}

/*
================
idTypeDef::Type
================
*/
etype_t idTypeDef::Type(void) const
{
	return type;
}

/*
================
idTypeDef::Size
================
*/
int idTypeDef::Size(void) const
{
	return size;
}

/*
================
idTypeDef::SuperClass

If type is an object, then returns the object's superclass
================
*/
idTypeDef *idTypeDef::SuperClass(void) const
{
	if (type != ev_object) {
		throw idCompileError("idTypeDef::SuperClass : tried to get superclass of a non-object type");
	}

	return auxType;
}

/*
================
idTypeDef::ReturnType

If type is a function, then returns the function's return type
================
*/
idTypeDef *idTypeDef::ReturnType(void) const
{
	if (type != ev_function) {
		throw idCompileError("idTypeDef::ReturnType: tried to get return type on non-function type");
	}

	return auxType;
}

/*
================
idTypeDef::SetReturnType

If type is a function, then sets the function's return type
================
*/
void idTypeDef::SetReturnType(idTypeDef *returntype)
{
	if (type != ev_function) {
		throw idCompileError("idTypeDef::SetReturnType: tried to set return type on non-function type");
	}

	auxType = returntype;
}

/*
================
idTypeDef::FieldType

If type is a field, then returns it's type
================
*/
idTypeDef *idTypeDef::FieldType(void) const
{
	if (type != ev_field) {
		throw idCompileError("idTypeDef::FieldType: tried to get field type on non-field type");
	}

	return auxType;
}

/*
================
idTypeDef::SetFieldType

If type is a field, then sets the function's return type
================
*/
void idTypeDef::SetFieldType(idTypeDef *fieldtype)
{
	if (type != ev_field) {
		throw idCompileError("idTypeDef::SetFieldType: tried to set return type on non-function type");
	}

	auxType = fieldtype;
}

/*
================
idTypeDef::PointerType

If type is a pointer, then returns the type it points to
================
*/
idTypeDef *idTypeDef::PointerType(void) const
{
	if (type != ev_pointer) {
		throw idCompileError("idTypeDef::PointerType: tried to get pointer type on non-pointer");
	}

	return auxType;
}

/*
================
idTypeDef::SetPointerType

If type is a pointer, then sets the pointer's type
================
*/
void idTypeDef::SetPointerType(idTypeDef *pointertype)
{
	if (type != ev_pointer) {
		throw idCompileError("idTypeDef::SetPointerType: tried to set type on non-pointer");
	}

	auxType = pointertype;
}

/*
================
idTypeDef::NumParameters
================
*/
int idTypeDef::NumParameters(void) const
{
	return parmTypes.Num();
}

/*
================
idTypeDef::GetParmType
================
*/
idTypeDef *idTypeDef::GetParmType(int parmNumber) const
{
	assert(parmNumber >= 0);
	assert(parmNumber < parmTypes.Num());
	return parmTypes[ parmNumber ];
}

/*
================
idTypeDef::GetParmName
================
*/
const char *idTypeDef::GetParmName(int parmNumber) const
{
	assert(parmNumber >= 0);
	assert(parmNumber < parmTypes.Num());
	return parmNames[ parmNumber ];
}

/*
================
idTypeDef::NumFunctions
================
*/
int idTypeDef::NumFunctions(void) const
{
	return functions.Num();
}

/*
================
idTypeDef::GetFunctionNumber
================
*/
int idTypeDef::GetFunctionNumber(const function_t *func) const
{
	int i;

	for (i = 0; i < functions.Num(); i++) {
		if (functions[ i ] == func) {
			return i;
		}
	}

	return -1;
}

/*
================
idTypeDef::GetFunction
================
*/
const function_t *idTypeDef::GetFunction(int funcNumber) const
{
	assert(funcNumber >= 0);
	assert(funcNumber < functions.Num());
	return functions[ funcNumber ];
}

/*
================
idTypeDef::AddFunction
================
*/
void idTypeDef::AddFunction(const function_t *func)
{
	int i;

	for (i = 0; i < functions.Num(); i++) {
		if (!strcmp(functions[ i ]->def->Name(), func->def->Name())) {
			if (func->def->TypeDef()->MatchesVirtualFunction(*functions[ i ]->def->TypeDef())) {
				functions[ i ] = func;
				return;
			}
		}
	}

	functions.Append(func);
}

/***********************************************************************

  idVarDef

***********************************************************************/

/*
================
idVarDef::idVarDef()
================
*/
idVarDef::idVarDef(idTypeDef *typeptr)
{
	typeDef		= typeptr;
	num			= 0;
	scope		= NULL;
	numUsers	= 0;
	initialized = idVarDef::uninitialized;
	memset(&value, 0, sizeof(value));
	name		= NULL;
	next		= NULL;
}

/*
============
idVarDef::~idVarDef
============
*/
idVarDef::~idVarDef()
{
	if (name) {
		name->RemoveDef(this);
	}
}

/*
============
idVarDef::Name
============
*/
const char *idVarDef::Name(void) const
{
	return name->Name();
}

/*
============
idVarDef::GlobalName
============
*/
const char *idVarDef::GlobalName(void) const
{
	if (scope != &def_namespace) {
		return va("%s::%s", scope->GlobalName(), name->Name());
	} else {
		return name->Name();
	}
}

/*
============
idVarDef::DepthOfScope
============
*/
int idVarDef::DepthOfScope(const idVarDef *otherScope) const
{
	const idVarDef *def;
	int depth;

	depth = 1;

	for (def = otherScope; def != NULL; def = def->scope) {
		if (def == scope) {
			return depth;
		}

		depth++;
	}

	return 0;
}

/*
============
idVarDef::SetFunction
============
*/
void idVarDef::SetFunction(function_t *func)
{
	assert(typeDef);
	initialized = initializedConstant;
	assert(typeDef->Type() == ev_function);
	value.functionPtr = func;
}

/*
============
idVarDef::SetObject
============
*/
void idVarDef::SetObject(idScriptObject *object)
{
	assert(typeDef);
	initialized = initialized;
	assert(typeDef->Inherits(&type_object));
	*value.objectPtrPtr = object;
}

/*
============
idVarDef::SetValue
============
*/
void idVarDef::SetValue(const eval_t &_value, bool constant)
{
	assert(typeDef);

	if (constant) {
		initialized = initializedConstant;
	} else {
		initialized = initializedVariable;
	}

	switch (typeDef->Type()) {
		case ev_pointer :
		case ev_boolean :
		case ev_field :
			*value.intPtr = _value._int;
			break;

		case ev_jumpoffset :
			value.jumpOffset = _value._int;
			break;

		case ev_argsize :
			value.argSize = _value._int;
			break;

		case ev_entity :
			*value.entityNumberPtr = _value.entity;
			break;

		case ev_string :
			idStr::Copynz(value.stringPtr, _value.stringPtr, MAX_STRING_LEN);
			break;

		case ev_float :
			*value.floatPtr = _value._float;
			break;

		case ev_vector :
			value.vectorPtr->x = _value.vector[ 0 ];
			value.vectorPtr->y = _value.vector[ 1 ];
			value.vectorPtr->z = _value.vector[ 2 ];
			break;

		case ev_function :
			value.functionPtr = _value.function;
			break;

		case ev_virtualfunction :
			value.virtualFunction = _value._int;
			break;

		case ev_object :
			*value.entityNumberPtr = _value.entity;
			break;

		default :
			throw idCompileError(va("weird type on '%s'", Name()));
			break;
	}
}

/*
============
idVarDef::SetString
============
*/
void idVarDef::SetString(const char *string, bool constant)
{
	if (constant) {
		initialized = initializedConstant;
	} else {
		initialized = initializedVariable;
	}

	assert(typeDef && (typeDef->Type() == ev_string));
	idStr::Copynz(value.stringPtr, string, MAX_STRING_LEN);
}

/*
============
idVarDef::PrintInfo
============
*/
void idVarDef::PrintInfo(idFile *file, int instructionPointer) const
{
	statement_t	*jumpst;
	int			jumpto;
	etype_t		etype;
	int			i;
	int			len;
	const char	*ch;

	if (initialized == initializedConstant) {
		file->Printf("const ");
	}

	etype = typeDef->Type();

	switch (etype) {
		case ev_jumpoffset :
			jumpto = instructionPointer + value.jumpOffset;
			jumpst = &gameLocal.program.GetStatement(jumpto);
			file->Printf("address %d [%s(%d)]", jumpto, gameLocal.program.GetFilename(jumpst->file), jumpst->linenumber);
			break;

		case ev_function :

			if (value.functionPtr->eventdef) {
				file->Printf("event %s", GlobalName());
			} else {
				file->Printf("function %s", GlobalName());
			}

			break;

		case ev_field :
			file->Printf("field %d", value.ptrOffset);
			break;

		case ev_argsize:
			file->Printf("args %d", value.argSize);
			break;

		default:
			file->Printf("%s ", typeDef->Name());

			if (initialized == initializedConstant) {
				switch (etype) {
					case ev_string :
						file->Printf("\"");
						len = strlen(value.stringPtr);
						ch = value.stringPtr;

						for (i = 0; i < len; i++, ch++) {
							if (idStr::CharIsPrintable(*ch)) {
								file->Printf("%c", *ch);
							} else if (*ch == '\n') {
								file->Printf("\\n");
							} else {
								file->Printf("\\x%.2x", static_cast<int>(*ch));
							}
						}

						file->Printf("\"");
						break;

					case ev_vector :
						file->Printf("'%s'", value.vectorPtr->ToString());
						break;

					case ev_float :
						file->Printf("%f", *value.floatPtr);
						break;

					case ev_virtualfunction :
						file->Printf("vtable[ %d ]", value.virtualFunction);
						break;

					default :
						file->Printf("%d", *value.intPtr);
						break;
				}
			} else if (initialized == stackVariable) {
				file->Printf("stack[%d]", value.stackOffset);
			} else {
				file->Printf("global[%d]", num);
			}

			break;
	}
}

/***********************************************************************

  idVarDef

***********************************************************************/

/*
============
idVarDefName::AddDef
============
*/
void idVarDefName::AddDef(idVarDef *def)
{
	assert(def->next == NULL);
	def->name = this;
	def->next = defs;
	defs = def;
}

/*
============
idVarDefName::RemoveDef
============
*/
void idVarDefName::RemoveDef(idVarDef *def)
{
	if (defs == def) {
		defs = def->next;
	} else {
		for (idVarDef *d = defs; d->next != NULL; d = d->next) {
			if (d->next == def) {
				d->next = def->next;
				break;
			}
		}
	}

	def->next = NULL;
	def->name = NULL;
}

/***********************************************************************

  idScriptObject

***********************************************************************/

/*
============
idScriptObject::idScriptObject
============
*/
idScriptObject::idScriptObject()
{
	data = NULL;
	type = &type_object;
}

/*
============
idScriptObject::~idScriptObject
============
*/
idScriptObject::~idScriptObject()
{
	Free();
}

/*
============
idScriptObject::Free
============
*/
void idScriptObject::Free(void)
{
	if (data) {
		Mem_Free(data);
	}

	data = NULL;
	type = &type_object;
}

/*
================
idScriptObject::Save
================
*/
void idScriptObject::Save(idSaveGame *savefile) const
{
	int size;

	if (type == &type_object && data == NULL) {
		// Write empty string for uninitialized object
		savefile->WriteString("");
	} else {
		savefile->WriteString(type->Name());
		size = type->Size();
		savefile->WriteInt(size);
		savefile->Write(data, size);
	}
}

/*
================
idScriptObject::Restore
================
*/
void idScriptObject::Restore(idRestoreGame *savefile)
{
	idStr typeName;
	int size;

	savefile->ReadString(typeName);

	// Empty string signals uninitialized object
	if (typeName.Length() == 0) {
		return;
	}

	if (!SetType(typeName)) {
		savefile->Error("idScriptObject::Restore: failed to restore object of type '%s'.", typeName.c_str());
	}

	savefile->ReadInt(size);

	if (size != type->Size()) {
		savefile->Error("idScriptObject::Restore: size of object '%s' doesn't match size in save game.", typeName.c_str());
	}

	savefile->Read(data, size);
}

/*
============
idScriptObject::SetType

Allocates an object and initializes memory.
============
*/
bool idScriptObject::SetType(const char *typeName)
{
	size_t size;
	idTypeDef *newtype;

	// lookup the type
	newtype = gameLocal.program.FindType(typeName);

	// only allocate memory if the object type changes
	if (newtype != type) {
		Free();

		if (!newtype) {
			gameLocal.Warning("idScriptObject::SetType: Unknown type '%s'", typeName);
			return false;
		}

		if (!newtype->Inherits(&type_object)) {
			gameLocal.Warning("idScriptObject::SetType: Can't create object of type '%s'.  Must be an object type.", newtype->Name());
			return false;
		}

		// set the type
		type = newtype;

		// allocate the memory
		size = type->Size();
		data = (byte *)Mem_Alloc(size);
	}

	// init object memory
	ClearObject();

	return true;
}

/*
============
idScriptObject::ClearObject

Resets the memory for the script object without changing its type.
============
*/
void idScriptObject::ClearObject(void)
{
	size_t size;

	if (type != &type_object) {
		// init object memory
		size = type->Size();
		memset(data, 0, size);
	}
}

/*
============
idScriptObject::HasObject
============
*/
bool idScriptObject::HasObject(void) const
{
	return (type != &type_object);
}

/*
============
idScriptObject::GetTypeDef
============
*/
idTypeDef *idScriptObject::GetTypeDef(void) const
{
	return type;
}

/*
============
idScriptObject::GetTypeName
============
*/
const char *idScriptObject::GetTypeName(void) const
{
	return type->Name();
}

/*
============
idScriptObject::GetConstructor
============
*/
const function_t *idScriptObject::GetConstructor(void) const
{
	const function_t *func;

	func = GetFunction("init");
	return func;
}

/*
============
idScriptObject::GetDestructor
============
*/
const function_t *idScriptObject::GetDestructor(void) const
{
	const function_t *func;

	func = GetFunction("destroy");
	return func;
}

/*
============
idScriptObject::GetFunction
============
*/
const function_t *idScriptObject::GetFunction(const char *name) const
{
	const function_t *func;

	if (type == &type_object) {
		return NULL;
	}

	func = gameLocal.program.FindFunction(name, type);
	return func;
}

/*
============
idScriptObject::GetVariable
============
*/
byte *idScriptObject::GetVariable(const char *name, etype_t etype) const
{
	int				i;
	int				pos;
	const idTypeDef	*t;
	const idTypeDef	*parm;

	if (type == &type_object) {
		return NULL;
	}

	t = type;

	do {
		if (t->SuperClass() != &type_object) {
			pos = t->SuperClass()->Size();
		} else {
			pos = 0;
		}

		for (i = 0; i < t->NumParameters(); i++) {
			parm = t->GetParmType(i);

			if (!strcmp(t->GetParmName(i), name)) {
				if (etype != parm->FieldType()->Type()) {
					return NULL;
				}

				return &data[ pos ];
			}

			if (parm->FieldType()->Inherits(&type_object)) {
				pos += type_object.Size();
			} else {
				pos += parm->FieldType()->Size();
			}
		}

		t = t->SuperClass();
	} while (t && (t != &type_object));

	return NULL;
}

/***********************************************************************

  idProgram

***********************************************************************/

/*
============
idProgram::AllocType
============
*/
idTypeDef *idProgram::AllocType(idTypeDef &type)
{
	idTypeDef *newtype;

	newtype	= new idTypeDef(type);
	types.Append(newtype);

	return newtype;
}

/*
============
idProgram::AllocType
============
*/
idTypeDef *idProgram::AllocType(etype_t etype, idVarDef *edef, const char *ename, int esize, idTypeDef *aux)
{
	idTypeDef *newtype;

	newtype	= new idTypeDef(etype, edef, ename, esize, aux);
	types.Append(newtype);

	return newtype;
}

/*
============
idProgram::GetType

Returns a preexisting complex type that matches the parm, or allocates
a new one and copies it out.
============
*/
idTypeDef *idProgram::GetType(idTypeDef &type, bool allocate)
{
	int i;

	//FIXME: linear search == slow
	for (i = types.Num() - 1; i >= 0; i--) {
		if (types[ i ]->MatchesType(type) && !strcmp(types[ i ]->Name(), type.Name())) {
			return types[ i ];
		}
	}

	if (!allocate) {
		return NULL;
	}

	// allocate a new one
	return AllocType(type);
}

/*
============
idProgram::FindType

Returns a preexisting complex type that matches the name, or returns NULL if not found
============
*/
idTypeDef *idProgram::FindType(const char *name)
{
	idTypeDef	*check;
	int			i;

	for (i = types.Num() - 1; i >= 0; i--) {
		check = types[ i ];

		if (!strcmp(check->Name(), name)) {
			return check;
		}
	}

	return NULL;
}

/*
============
idProgram::GetDefList
============
*/
idVarDef *idProgram::GetDefList(const char *name) const
{
	int i, hash;

	hash = varDefNameHash.GenerateKey(name, true);

	for (i = varDefNameHash.First(hash); i != -1; i = varDefNameHash.Next(i)) {
		if (idStr::Cmp(varDefNames[i]->Name(), name) == 0) {
			return varDefNames[i]->GetDefs();
		}
	}

	return NULL;
}

/*
============
idProgram::AddDefToNameList
============
*/
void idProgram::AddDefToNameList(idVarDef *def, const char *name)
{
	int i, hash;

	hash = varDefNameHash.GenerateKey(name, true);

	for (i = varDefNameHash.First(hash); i != -1; i = varDefNameHash.Next(i)) {
		if (idStr::Cmp(varDefNames[i]->Name(), name) == 0) {
			break;
		}
	}

	if (i == -1) {
		i = varDefNames.Append(new idVarDefName(name));
		varDefNameHash.Add(hash, i);
	}

	varDefNames[i]->AddDef(def);
}


/*
============
idProgram::AllocMem
============
*/
byte *idProgram::AllocMem(size_t size)
{
	byte *bytePtr;

	bytePtr = &variables[ numVariables ];
	numVariables += size;

	if (numVariables > sizeof(variables)) {
		throw idCompileError(va("Exceeded global memory size (%zd bytes)", sizeof(variables)));
	}

	memset(bytePtr, 0, size);

	return bytePtr;
}

/*
============
idProgram::AllocVarDef
============
*/
idVarDef *idProgram::AllocVarDef(idTypeDef *type, const char *name, idVarDef *scope)
{
	idVarDef	*def;

	// allocate a new def
	def = new idVarDef(type);
	def->scope		= scope;
	def->numUsers	= 1;
	def->num		= varDefs.Append(def);

	// add the def to the list with defs with this name and set the name pointer
	AddDefToNameList(def, name);

	return def;
}

/*
============
idProgram::AllocDef
============
*/
idVarDef *idProgram::AllocDef(idTypeDef *type, const char *name, idVarDef *scope, bool constant)
{
	idVarDef	*def;
	idStr		element;
	idVarDef	*def_x;
	idVarDef	*def_y;
	idVarDef	*def_z;

	// allocate a new def
	def = AllocVarDef(type, name, scope);

	if ((type->Type() == ev_vector) || ((type->Type() == ev_field) && (type->FieldType()->Type() == ev_vector))) {
		//
		// vector
		//
		if (!strcmp(name, RESULT_STRING)) {
			// <RESULT> vector defs don't need the _x, _y and _z components
			assert(scope->Type() == ev_function);
			def->value.stackOffset	= scope->value.functionPtr->locals;
			def->initialized		= idVarDef::stackVariable;
			scope->value.functionPtr->locals += type->Size();
		} else if (scope->TypeDef()->Inherits(&type_object)) {
			idTypeDef	newtype(ev_field, NULL, "float field", 0, &type_float);
			idTypeDef	*_type = GetType(newtype, true);

			// set the value to the variable's position in the object
			def->value.ptrOffset = scope->TypeDef()->Size();

			// make automatic defs for the vectors elements
			// origin can be accessed as origin_x, origin_y, and origin_z
			sprintf(element, "%s_x", def->Name());
			def_x = AllocDef(_type, element, scope, constant);

			sprintf(element, "%s_y", def->Name());
			def_y = AllocDef(_type, element, scope, constant);
			def_y->value.ptrOffset = def_x->value.ptrOffset + sizeof(float);

			sprintf(element, "%s_z", def->Name());
			def_z = AllocDef(_type, element, scope, constant);
			def_z->value.ptrOffset = def_y->value.ptrOffset + sizeof(float);
		} else {
			idTypeDef	newtype(ev_float, &def_float, "float vector", 0, NULL);
			idTypeDef	*_type = GetType(newtype, true);

			// make automatic defs for the vectors elements
			// origin can be accessed as origin_x, origin_y, and origin_z
			sprintf(element, "%s_x", def->Name());
			def_x = AllocVarDef(_type, element, scope);

			sprintf(element, "%s_y", def->Name());
			def_y = AllocVarDef(_type, element, scope);

			sprintf(element, "%s_z", def->Name());
			def_z = AllocVarDef(_type, element, scope);

			// point the vector def to the coordinates
			if (scope->Type() == ev_function) {
				//
				// stack variable
				//
				def->value.stackOffset	= scope->value.functionPtr->locals;
				def->initialized		= idVarDef::stackVariable;

				scope->value.functionPtr->locals += type->Size();

				def_x->value.stackOffset = def->value.stackOffset;
				def_y->value.stackOffset = def_x->value.stackOffset + sizeof(float);
				def_z->value.stackOffset = def_y->value.stackOffset + sizeof(float);
			} else {
				//
				// global variable
				//
				def->value.bytePtr = AllocMem(type->Size());
				def_x->value.bytePtr = def->value.bytePtr;
				def_y->value.bytePtr = def_x->value.bytePtr + sizeof(float);
				def_z->value.bytePtr = def_y->value.bytePtr + sizeof(float);
			}

			def_x->initialized = def->initialized;
			def_y->initialized = def->initialized;
			def_z->initialized = def->initialized;
		}
	} else if (scope->TypeDef()->Inherits(&type_object)) {
		//
		// object variable
		//
		// set the value to the variable's position in the object
		def->value.ptrOffset = scope->TypeDef()->Size();
	} else if (scope->Type() == ev_function) {
		//
		// stack variable
		//
		// since we don't know how many local variables there are,
		// we have to have them go backwards on the stack
		def->value.stackOffset	= scope->value.functionPtr->locals;
		def->initialized		= idVarDef::stackVariable;

		if (type->Inherits(&type_object)) {
			// objects only have their entity number on the stack, not the entire object
			scope->value.functionPtr->locals += type_object.Size();
		} else {
			scope->value.functionPtr->locals += type->Size();
		}
	} else {
		//
		// global variable
		//
		def->value.bytePtr = AllocMem(def->TypeDef()->Size());
	}

	return def;
}

/*
============
idProgram::GetDef

If type is NULL, it will match any type
============
*/
idVarDef *idProgram::GetDef(const idTypeDef *type, const char *name, const idVarDef *scope) const
{
	idVarDef		*def;
	idVarDef		*bestDef;
	int				bestDepth;
	int				depth;

	bestDepth = 0;
	bestDef = NULL;

	for (def = GetDefList(name); def != NULL; def = def->Next()) {
		if (def->scope->Type() == ev_namespace) {
			depth = def->DepthOfScope(scope);

			if (!depth) {
				// not in the same namespace
				continue;
			}
		} else if (def->scope != scope) {
			// in a different function
			continue;
		} else {
			depth = 1;
		}

		if (!bestDef || (depth < bestDepth)) {
			bestDepth = depth;
			bestDef = def;
		}
	}

	// see if the name is already in use for another type
	if (bestDef && type && (bestDef->TypeDef() != type)) {
		throw idCompileError(va("Type mismatch on redeclaration of %s", name));
	}

	return bestDef;
}

/*
============
idProgram::FreeDef
============
*/
void idProgram::FreeDef(idVarDef *def, const idVarDef *scope)
{
	idVarDef *e;
	int i;

	if (def->Type() == ev_vector) {
		idStr name;

		sprintf(name, "%s_x", def->Name());
		e = GetDef(NULL, name, scope);

		if (e) {
			FreeDef(e, scope);
		}

		sprintf(name, "%s_y", def->Name());
		e = GetDef(NULL, name, scope);

		if (e) {
			FreeDef(e, scope);
		}

		sprintf(name, "%s_z", def->Name());
		e = GetDef(NULL, name, scope);

		if (e) {
			FreeDef(e, scope);
		}
	}

	varDefs.RemoveIndex(def->num);

	for (i = def->num; i < varDefs.Num(); i++) {
		varDefs[ i ]->num = i;
	}

	delete def;
}

/*
============
idProgram::FindFreeResultDef
============
*/
idVarDef *idProgram::FindFreeResultDef(idTypeDef *type, const char *name, idVarDef *scope, const idVarDef *a, const idVarDef *b)
{
	idVarDef *def;

	for (def = GetDefList(name); def != NULL; def = def->Next()) {
		if (def == a || def == b) {
			continue;
		}

		if (def->TypeDef() != type) {
			continue;
		}

		if (def->scope != scope) {
			continue;
		}

		if (def->numUsers <= 1) {
			continue;
		}

		return def;
	}

	return AllocDef(type, name, scope, false);
}

/*
================
idProgram::FindFunction

Searches for the specified function in the currently loaded script.  A full namespace should be
specified if not in the global namespace.

Returns 0 if function not found.
Returns >0 if function found.
================
*/
function_t *idProgram::FindFunction(const char *name) const
{
	int			start;
	int			pos;
	idVarDef	*namespaceDef;
	idVarDef	*def;

	assert(name);

	idStr fullname = name;
	start = 0;
	namespaceDef = &def_namespace;

	do {
		pos = fullname.Find("::", true, start);

		if (pos < 0) {
			break;
		}

		idStr namespaceName = fullname.Mid(start, pos - start);
		def = GetDef(NULL, namespaceName, namespaceDef);

		if (!def) {
			// couldn't find namespace
			return NULL;
		}

		namespaceDef = def;

		// skip past the ::
		start = pos + 2;
	} while (def->Type() == ev_namespace);

	idStr funcName = fullname.Right(fullname.Length() - start);
	def = GetDef(NULL, funcName, namespaceDef);

	if (!def) {
		// couldn't find function
		return NULL;
	}

	if ((def->Type() == ev_function) && (def->value.functionPtr->eventdef == NULL)) {
		return def->value.functionPtr;
	}

	// is not a function, or is an eventdef
	return NULL;
}

/*
================
idProgram::FindFunction

Searches for the specified object function in the currently loaded script.

Returns 0 if function not found.
Returns >0 if function found.
================
*/
function_t *idProgram::FindFunction(const char *name, const idTypeDef *type) const
{
	const idVarDef	*tdef;
	const idVarDef	*def;

	// look for the function
	def = NULL;

	for (tdef = type->def; tdef != &def_object; tdef = tdef->TypeDef()->SuperClass()->def) {
		def = GetDef(NULL, name, tdef);

		if (def) {
			return def->value.functionPtr;
		}
	}

	return NULL;
}

/*
================
idProgram::AllocFunction
================
*/
function_t &idProgram::AllocFunction(idVarDef *def)
{
	if (functions.Num() >= functions.Max()) {
		throw idCompileError(va("Exceeded maximum allowed number of functions (%d)", functions.Max()));
	}

	// fill in the dfunction
	function_t &func	= *functions.Alloc();
	func.eventdef		= NULL;
	func.def			= def;
	func.type			= def->TypeDef();
	func.firstStatement	= 0;
	func.numStatements	= 0;
	func.parmTotal		= 0;
	func.locals			= 0;
	func.filenum		= filenum;
	func.parmSize.SetGranularity(1);
	func.SetName(def->GlobalName());

	def->SetFunction(&func);

	return func;
}

/*
================
idProgram::SetEntity
================
*/
void idProgram::SetEntity(const char *name, idEntity *ent)
{
	idVarDef	*def;
	idStr		defName("$");

	defName += name;

	def = GetDef(&type_entity, defName, &def_namespace);

	if (def && (def->initialized != idVarDef::stackVariable)) {
		// 0 is reserved for NULL entity
		if (!ent) {
			*def->value.entityNumberPtr = 0;
		} else {
			*def->value.entityNumberPtr = ent->entityNumber + 1;
		}
	}
}

/*
================
idProgram::AllocStatement
================
*/
statement_t *idProgram::AllocStatement(void)
{
	if (statements.Num() >= statements.Max()) {
		throw idCompileError(va("Exceeded maximum allowed number of statements (%d)", statements.Max()));
	}

	return statements.Alloc();
}

/*
==============
idProgram::BeginCompilation

called before compiling a batch of files, clears the pr struct
==============
*/
void idProgram::BeginCompilation(void)
{
	statement_t	*statement;

	FreeData();

	try {
		// make the first statement a return for a "NULL" function
		statement = AllocStatement();
		statement->linenumber	= 0;
		statement->file 		= 0;
		statement->op			= OP_RETURN;
		statement->a			= NULL;
		statement->b			= NULL;
		statement->c			= NULL;

		// define NULL
		//AllocDef( &type_void, "<NULL>", &def_namespace, true );

		// define the return def
		returnDef = AllocDef(&type_vector, "<RETURN>", &def_namespace, false);

		// define the return def for strings
		returnStringDef = AllocDef(&type_string, "<RETURN>", &def_namespace, false);

		// define the sys object
		sysDef = AllocDef(&type_void, "sys", &def_namespace, true);
	}

	catch (idCompileError &err) {
		gameLocal.Error("%s", err.error);
	}
}

/*
==============
idProgram::DisassembleStatement
==============
*/
void idProgram::DisassembleStatement(idFile *file, int instructionPointer) const
{
	const opcode_t			*op;
	const statement_t	*statement;

	statement = &statements[ instructionPointer ];
	op = &idCompiler::opcodes[ statement->op ];
	file->Printf("%20s(%d):\t%6d: %15s\t", fileList[ statement->file ].c_str(), statement->linenumber, instructionPointer, op->opname);

	if (statement->a) {
		file->Printf("\ta: ");
		statement->a->PrintInfo(file, instructionPointer);
	}

	if (statement->b) {
		file->Printf("\tb: ");
		statement->b->PrintInfo(file, instructionPointer);
	}

	if (statement->c) {
		file->Printf("\tc: ");
		statement->c->PrintInfo(file, instructionPointer);
	}

	file->Printf("\n");
}

/*
==============
idProgram::Disassemble
==============
*/
void idProgram::Disassemble(void) const
{
	int					i;
	int					instructionPointer;
	const function_t	*func;
	idFile				*file;

	file = fileSystem->OpenFileByMode("script/disasm.txt", FS_WRITE);

	for (i = 0; i < functions.Num(); i++) {
		func = &functions[ i ];

		if (func->eventdef) {
			// skip eventdefs
			continue;
		}

		file->Printf("\nfunction %s() %d stack used, %d parms, %d locals {\n", func->Name(), func->locals, func->parmTotal, func->locals - func->parmTotal);

		for (instructionPointer = 0; instructionPointer < func->numStatements; instructionPointer++) {
			DisassembleStatement(file, func->firstStatement + instructionPointer);
		}

		file->Printf("}\n");
	}

	fileSystem->CloseFile(file);
}

/*
==============
idProgram::FinishCompilation

Called after all files are compiled to check for errors
==============
*/
void idProgram::FinishCompilation(void)
{
	int	i;

	top_functions	= functions.Num();
	top_statements	= statements.Num();
	top_types		= types.Num();
	top_defs		= varDefs.Num();
	top_files		= fileList.Num();

	variableDefaults.Clear();
	variableDefaults.SetNum(numVariables);

	for (i = 0; i < numVariables; i++) {
		variableDefaults[ i ] = variables[ i ];
	}
}

/*
==============
idProgram::CompileStats

called after all files are compiled to report memory usage.
==============
*/
void idProgram::CompileStats(void)
{
	int	memused;
	int	memallocated;
	int	numdefs;
	int	stringspace;
	int funcMem;
	int	i;

	gameLocal.Printf("---------- Compile stats ----------\n");
	gameLocal.DPrintf("Files loaded:\n");

	stringspace = 0;

	for (i = 0; i < fileList.Num(); i++) {
		gameLocal.DPrintf("   %s\n", fileList[ i ].c_str());
		stringspace += fileList[ i ].Allocated();
	}

	stringspace += fileList.Size();

	numdefs = varDefs.Num();
	memused = varDefs.Num() * sizeof(idVarDef);
	memused += types.Num() * sizeof(idTypeDef);
	memused += stringspace;

	for (i = 0; i < types.Num(); i++) {
		memused += types[ i ]->Allocated();
	}

	funcMem = functions.MemoryUsed();

	for (i = 0; i < functions.Num(); i++) {
		funcMem += functions[ i ].Allocated();
	}

	memallocated = funcMem + memused + sizeof(idProgram);

	memused += statements.MemoryUsed();
	memused += functions.MemoryUsed();	// name and filename of functions are shared, so no need to include them
	memused += sizeof(variables);

	gameLocal.Printf("\nMemory usage:\n");
	gameLocal.Printf("     Strings: %d, %d bytes\n", fileList.Num(), stringspace);
	gameLocal.Printf("  Statements: %d, %zd bytes\n", statements.Num(), statements.MemoryUsed());
	gameLocal.Printf("   Functions: %d, %d bytes\n", functions.Num(), funcMem);
	gameLocal.Printf("   Variables: %d bytes\n", numVariables);
	gameLocal.Printf("    Mem used: %d bytes\n", memused);
	gameLocal.Printf(" Static data: %zd bytes\n", sizeof(idProgram));
	gameLocal.Printf("   Allocated: %d bytes\n", memallocated);
	gameLocal.Printf(" Thread size: %zd bytes\n\n", sizeof(idThread));
}

/*
================
idProgram::CompileText
================
*/
bool idProgram::CompileText(const char *source, const char *text, bool console)
{
	idCompiler	compiler;
	int			i;
	idVarDef	*def;
	idStr		ospath;

	// use a full os path for GetFilenum since it calls OSPathToRelativePath to convert filenames from the parser
	ospath = fileSystem->RelativePathToOSPath(source);
	filenum = GetFilenum(ospath);

	try {
		compiler.CompileFile(text, filename, console);

		// check to make sure all functions prototyped have code
		for (i = 0; i < varDefs.Num(); i++) {
			def = varDefs[ i ];

			if ((def->Type() == ev_function) && ((def->scope->Type() == ev_namespace) || def->scope->TypeDef()->Inherits(&type_object))) {
				if (!def->value.functionPtr->eventdef && !def->value.functionPtr->firstStatement) {
					throw idCompileError(va("function %s was not defined\n", def->GlobalName()));
				}
			}
		}
	}

	catch (idCompileError &err) {
		if (console) {
			gameLocal.Printf("%s\n", err.error);
			return false;
		} else {
			gameLocal.Error("%s\n", err.error);
		}
	};

	if (!console) {
		CompileStats();
	}

	return true;
}

/*
================
idProgram::CompileFunction
================
*/
const function_t *idProgram::CompileFunction(const char *functionName, const char *text)
{
	bool result;

	result = CompileText(functionName, text, false);

	if (g_disasm.GetBool()) {
		Disassemble();
	}

	if (!result) {
		gameLocal.Error("Compile failed.");
	}

	return FindFunction(functionName);
}

/*
================
idProgram::CompileFile
================
*/
void idProgram::CompileFile(const char *filename)
{
	char *src;
	bool result;

	if (fileSystem->ReadFile(filename, (void **)&src, NULL) < 0) {
		gameLocal.Error("Couldn't load %s\n", filename);
	}

	result = CompileText(filename, src, false);

	fileSystem->FreeFile(src);

	if (g_disasm.GetBool()) {
		Disassemble();
	}

	if (!result) {
		gameLocal.Error("Compile failed in file %s.", filename);
	}
}

/*
================
idProgram::FreeData
================
*/
void idProgram::FreeData(void)
{
	int i;

	// free the defs
	varDefs.DeleteContents(true);
	varDefNames.DeleteContents(true);
	varDefNameHash.Free();

	returnDef		= NULL;
	returnStringDef = NULL;
	sysDef			= NULL;

	// free any special types we've created
	types.DeleteContents(true);

	filenum = 0;

	numVariables = 0;
	memset(variables, 0, sizeof(variables));

	// clear all the strings in the functions so that it doesn't look like we're leaking memory.
	for (i = 0; i < functions.Num(); i++) {
		functions[ i ].Clear();
	}

	filename.Clear();
	fileList.Clear();
	statements.Clear();
	functions.Clear();

	top_functions	= 0;
	top_statements	= 0;
	top_types		= 0;
	top_defs		= 0;
	top_files		= 0;

	filename = "";
}

/*
================
idProgram::Startup
================
*/
void idProgram::Startup(const char *defaultScript)
{
	gameLocal.Printf("Initializing scripts\n");

	// make sure all data is freed up
	idThread::Restart();

	// get ready for loading scripts
	BeginCompilation();

	// load the default script
	if (defaultScript && *defaultScript) {
		CompileFile(defaultScript);
	}

	FinishCompilation();
}

/*
================
idProgram::Save
================
*/
void idProgram::Save(idSaveGame *savefile) const
{
	int i;
	int currentFileNum = top_files;

	savefile->WriteInt((fileList.Num() - currentFileNum));

	while (currentFileNum < fileList.Num()) {
		savefile->WriteString(fileList[ currentFileNum ]);
		currentFileNum++;
	}

	for (i = 0; i < variableDefaults.Num(); i++) {
		if (variables[i] != variableDefaults[i]) {
			savefile->WriteInt(i);
			savefile->WriteByte(variables[i]);
		}
	}

	// Mark the end of the diff with default variables with -1
	savefile->WriteInt(-1);

	savefile->WriteInt(numVariables);

	for (i = variableDefaults.Num(); i < numVariables; i++) {
		savefile->WriteByte(variables[i]);
	}

	int checksum = CalculateChecksum();
	savefile->WriteInt(checksum);
}

/*
================
idProgram::Restore
================
*/
bool idProgram::Restore(idRestoreGame *savefile)
{
	int i, num, index;
	bool result = true;
	idStr scriptname;

	savefile->ReadInt(num);

	for (i = 0; i < num; i++) {
		savefile->ReadString(scriptname);
		CompileFile(scriptname);
	}

	savefile->ReadInt(index);

	while (index >= 0) {
		savefile->ReadByte(variables[index]);
		savefile->ReadInt(index);
	}

	savefile->ReadInt(num);

	for (i = variableDefaults.Num(); i < num; i++) {
		savefile->ReadByte(variables[i]);
	}

	int saved_checksum, checksum;

	savefile->ReadInt(saved_checksum);
	checksum = CalculateChecksum();

	if (saved_checksum != checksum) {
		result = false;
	}

	return result;
}

/*
================
idProgram::CalculateChecksum
================
*/
int idProgram::CalculateChecksum(void) const
{
	int i, result;

	typedef struct {
		unsigned short	op;
		int				a;
		int				b;
		int				c;
		unsigned short	linenumber;
		unsigned short	file;
	} statementBlock_t;

	statementBlock_t	*statementList = new statementBlock_t[ statements.Num()];

	memset(statementList, 0, (sizeof(statementBlock_t) * statements.Num()));

	// Copy info into new list, using the variable numbers instead of a pointer to the variable
	for (i = 0; i < statements.Num(); i++) {
		statementList[i].op = statements[i].op;

		if (statements[i].a) {
			statementList[i].a = statements[i].a->num;
		} else {
			statementList[i].a = -1;
		}

		if (statements[i].b) {
			statementList[i].b = statements[i].b->num;
		} else {
			statementList[i].b = -1;
		}

		if (statements[i].c) {
			statementList[i].c = statements[i].c->num;
		} else {
			statementList[i].c = -1;
		}

		statementList[i].linenumber = statements[i].linenumber;
		statementList[i].file = statements[i].file;
	}

	result = MD4_BlockChecksum(statementList, (sizeof(statementBlock_t) * statements.Num()));

	delete [] statementList;

	return result;
}

/*
==============
idProgram::Restart

Restores all variables to their initial value
==============
*/
void idProgram::Restart(void)
{
	int i;

	idThread::Restart();

	//
	// since there may have been a script loaded by the map or the user may
	// have typed "script" from the console, free up any types and vardefs that
	// have been allocated after the initial startup
	//
	for (i = top_types; i < types.Num(); i++) {
		delete types[ i ];
	}

	types.SetNum(top_types, false);

	for (i = top_defs; i < varDefs.Num(); i++) {
		delete varDefs[ i ];
	}

	varDefs.SetNum(top_defs, false);

	for (i = top_functions; i < functions.Num(); i++) {
		functions[ i ].Clear();
	}

	functions.SetNum(top_functions);

	statements.SetNum(top_statements);
	fileList.SetNum(top_files, false);
	filename.Clear();

	// reset the variables to their default values
	numVariables = variableDefaults.Num();

	for (i = 0; i < numVariables; i++) {
		variables[ i ] = variableDefaults[ i ];
	}
}

/*
================
idProgram::GetFilenum
================
*/
int idProgram::GetFilenum(const char *name)
{
	if (filename == name) {
		return filenum;
	}

	idStr strippedName;
	strippedName = fileSystem->OSPathToRelativePath(name);

	if (!strippedName.Length()) {
		// not off the base path so just use the full path
		filenum = fileList.AddUnique(name);
	} else {
		filenum = fileList.AddUnique(strippedName);
	}

	// save the unstripped name so that we don't have to strip the incoming name every time we call GetFilenum
	filename = name;

	return filenum;
}

/*
================
idProgram::idProgram
================
*/
idProgram::idProgram()
{
	FreeData();
}

/*
================
idProgram::~idProgram
================
*/
idProgram::~idProgram()
{
	FreeData();
}

/*
================
idProgram::ReturnEntity
================
*/
void idProgram::ReturnEntity(idEntity *ent)
{
	if (ent) {
		*returnDef->value.entityNumberPtr = ent->entityNumber + 1;
	} else {
		*returnDef->value.entityNumberPtr = 0;
	}
}