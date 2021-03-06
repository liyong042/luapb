#include <string>
#include "lua_pb.h"
#include "lua_log.h"
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/compiler/importer.h>
//////////////////////////////////////////////////////////
using namespace google::protobuf;
using namespace google::protobuf::compiler;
//---------------------------------------------------------------
class MyFileErrorCollector : public google::protobuf::compiler::MultiFileErrorCollector
{
	virtual void AddError(const std::string & filename,int line,int column, const std::string & message)
	{
		DF_LOG(LOG_ERROR, "Add File Error " << filename << " line:" << line << column << " :" << message);
	}
};
static int  st_pb_to_array(const Message &msg, const FieldDescriptor* pField, lua_State *L);
static int  st_pb_to_table(const Message &msg, lua_State *L);
static void st_table_to_pb(Message* msg, const FieldDescriptor* pField, lua_State* L);
//------------------------------------------------------------
static MyFileErrorCollector  g_errorCollector;
static DiskSourceTree        g_sourceTree;
static Importer              g_importer(&g_sourceTree, &g_errorCollector);
static DynamicMessageFactory g_factory; 
//===================protobuf to lua table ========================================
static int st_pb_to_value(const Message &msg, const FieldDescriptor* pField, lua_State *L)
{
	const Reflection* pRef = msg.GetReflection();
	switch (pField->cpp_type()) {
	case FieldDescriptor::CPPTYPE_BOOL:   { lua_pushboolean(L, pRef->GetBool(msg, pField));   break; }
	case FieldDescriptor::CPPTYPE_UINT32: { lua_pushinteger(L, pRef->GetUInt32(msg, pField)); break; }
	case FieldDescriptor::CPPTYPE_UINT64: { lua_pushinteger(L, pRef->GetUInt64(msg, pField)); break; }
	case FieldDescriptor::CPPTYPE_INT32:  { lua_pushinteger(L, pRef->GetInt32(msg, pField));  break; }
	case FieldDescriptor::CPPTYPE_INT64:  { lua_pushinteger(L, pRef->GetInt64(msg, pField));  break; }
	case FieldDescriptor::CPPTYPE_FLOAT:  { lua_pushnumber(L, static_cast<double>(pRef->GetFloat(msg, pField))); break; }
	case FieldDescriptor::CPPTYPE_DOUBLE: { lua_pushnumber(L, pRef->GetDouble(msg, pField));  break; }
	case FieldDescriptor::CPPTYPE_ENUM:   { lua_pushnumber(L, pRef->GetEnum(msg, pField)->number()); break; }
	case FieldDescriptor::CPPTYPE_STRING: {
		const std::string val = pRef->GetString(msg, pField); 
		lua_pushlstring(L, val.data(), val.size());
		break;
	}
	case FieldDescriptor::CPPTYPE_MESSAGE:{
		const Message& tmp = pRef->GetMessage(msg, pField);
		st_pb_to_table( tmp, L );
		break;
	}
	default:{ 
		lua_pushnil(L);
		DF_LOG(LOG_ERROR, " not support name:" << pField->name() << " type:" << pField->type_name());
		break;
	}
	}
	return 1;
}
static int st_pb_to_array(const Message &msg, const FieldDescriptor* pField, lua_State *L)
{
	lua_newtable(L);
	const Reflection* pRef = msg.GetReflection();
	const int		 size = pRef->FieldSize(msg, pField);
	for ( int i = 0; i < size; i++ ){
		lua_pushnumber(L, i+1); 
		switch (pField->cpp_type()) {
			case FieldDescriptor::CPPTYPE_BOOL:   { lua_pushboolean(L, pRef->GetRepeatedBool(msg, pField, i));   break; }
			case FieldDescriptor::CPPTYPE_UINT32: { lua_pushinteger(L, pRef->GetRepeatedUInt32(msg, pField, i)); break; }
			case FieldDescriptor::CPPTYPE_UINT64: { lua_pushinteger(L, pRef->GetRepeatedUInt64(msg, pField, i)); break; }
			case FieldDescriptor::CPPTYPE_INT32:  { lua_pushinteger(L, pRef->GetRepeatedInt32(msg, pField, i));  break; }
			case FieldDescriptor::CPPTYPE_INT64:  { lua_pushinteger(L, pRef->GetRepeatedInt64(msg, pField, i));  break; }
			case FieldDescriptor::CPPTYPE_FLOAT:  { lua_pushnumber(L, static_cast<double>(pRef->GetRepeatedFloat(msg, pField, i))); break; }
			case FieldDescriptor::CPPTYPE_DOUBLE: { lua_pushnumber(L, pRef->GetRepeatedDouble(msg, pField, i)); break; }
			case FieldDescriptor::CPPTYPE_STRING: {
				std::string val = pRef->GetRepeatedString(msg, pField, i); 
				lua_pushlstring(L, val.data(), val.size());
				break;
			}
			case FieldDescriptor::CPPTYPE_MESSAGE:{
				const Message& tmp = pRef->GetRepeatedMessage(msg, pField, i);
				st_pb_to_table(tmp, L);
				break;
			}
			case FieldDescriptor::CPPTYPE_ENUM:{ lua_pushnumber(L, pRef->GetRepeatedEnum(msg, pField, i)->number()); break; }
			default:{ 
				lua_pushnil(L);
				DF_LOG(LOG_ERROR, " not support name:" << pField->name() << " type:" << pField->type_name());
				break; 
			}
		}
		lua_settable(L, -3 );
	}
	return 1; 
}
static int st_pb_to_table(const Message &msg, lua_State *L)
{
	lua_newtable(L);
	std::vector<const FieldDescriptor*> field_list;
	const Reflection* pRef = msg.GetReflection();
	pRef->ListFields( msg, &field_list );
	for (unsigned i = 0; i < field_list.size(); ++i) {
		const FieldDescriptor* pfield = field_list[i];
		if ( pfield->is_repeated() ){
			st_pb_to_array( msg, pfield, L);
		}
		else{
			st_pb_to_value( msg, pfield, L);
		}
		lua_setfield(L, -2, pfield->name().c_str());
	}
	return 1; 
}
//=================lua table to protobuf ==============================================
//get pb enum type 
static  const EnumValueDescriptor*  st_value_get_etype(const FieldDescriptor* pField, lua_State* L)
{
	if (lua_type(L, -1) == LUA_TNUMBER){
		return pField->enum_type()->FindValueByNumber(lua_tonumber(L, -1));
	}
	return pField->enum_type()->FindValueByName(lua_tostring(L, -1));
}
//set pb value 
static  void st_value_to_pb(const Reflection* pRef, const FieldDescriptor* pField, Message *msg, lua_State* L)
{
	if( pField->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE ){
		if (lua_type(L, -1) == LUA_TTABLE){
			Message* pSubMsg = pRef->MutableMessage( msg, pField );
			st_table_to_pb( pSubMsg, NULL, L);
		}
		return;
	}
	if ( lua_type(L, -1) != LUA_TSTRING && lua_type(L, -1) != LUA_TNUMBER ){
		return;
	}
	switch (pField->cpp_type())
	{
		case FieldDescriptor::CPPTYPE_INT32: { pRef->SetInt32  (msg, pField, lua_tointeger(L, -1)); break;}
		case FieldDescriptor::CPPTYPE_UINT32:{ pRef->SetUInt32 (msg, pField, lua_tointeger(L, -1)); break;}
		case FieldDescriptor::CPPTYPE_BOOL:  { pRef->SetBool   (msg, pField, lua_toboolean(L, -1)); break;}
		case FieldDescriptor::CPPTYPE_INT64: { pRef->SetInt64  (msg, pField, lua_tonumber(L, -1) ); break;}
		case FieldDescriptor::CPPTYPE_UINT64:{ pRef->SetUInt64 (msg, pField, lua_tonumber(L, -1) ); break;}
		case FieldDescriptor::CPPTYPE_DOUBLE:{ pRef->SetDouble (msg, pField, lua_tonumber(L, -1) ); break;}
		case FieldDescriptor::CPPTYPE_FLOAT: { pRef->SetFloat  (msg, pField, lua_tonumber(L, -1) ); break;}
		case FieldDescriptor::CPPTYPE_ENUM:  { 
			const EnumValueDescriptor* pType = st_value_get_etype( pField, L);
			if (NULL == pType ) {
				DF_LOG(LOG_ERROR, "Fail to find enum descriptor name:" << lua_tostring(L, -1));
			}
			else {
				pRef->SetEnum(msg, pField, pType );
			}
			break;
		}
		case FieldDescriptor::CPPTYPE_STRING:{
			size_t len = 0;
			const char* c = lua_tolstring(L, -1, &len);
			pRef->SetString(msg, pField, std::string(c, len));
			break;
		}
		default:{
			DF_LOG(LOG_ERROR, " not support name:" << pField->name() << " type:" << pField->type_name() ) ; 
			break;
		}
	}
}
//add pb repaete value 
static  void st_value_to_pb_repeated(const Reflection* pRef, const FieldDescriptor* pField, Message *msg, lua_State* L)
{
	const int type = lua_type(L, -1);
	if ( type == LUA_TTABLE ){
		Message* subMessage = pRef->AddMessage(msg, pField);
		st_table_to_pb(subMessage, NULL, L);
	}
	if (type != LUA_TSTRING && type != LUA_TNUMBER){
		return;
	}
	switch (pField->cpp_type())
	{
		case FieldDescriptor::CPPTYPE_INT32: { pRef->AddInt32 (msg, pField, lua_tointeger(L, -1)); break; }
		case FieldDescriptor::CPPTYPE_INT64: { pRef->AddInt64 (msg, pField, lua_tointeger(L, -1)); break; }
		case FieldDescriptor::CPPTYPE_UINT32:{ pRef->AddUInt32(msg, pField, lua_tointeger(L, -1)); break; }
		case FieldDescriptor::CPPTYPE_UINT64:{ pRef->AddUInt64(msg, pField, lua_tointeger(L, -1)); break; }
		case FieldDescriptor::CPPTYPE_BOOL:  { pRef->AddBool  (msg, pField, lua_tointeger(L, -1)); break; }
		case FieldDescriptor::CPPTYPE_FLOAT: { pRef->AddFloat (msg, pField, lua_tonumber(L, -1));  break; }
		case FieldDescriptor::CPPTYPE_DOUBLE:{ pRef->AddDouble(msg, pField, lua_tonumber(L, -1));  break; }
		case FieldDescriptor::CPPTYPE_ENUM:  {
				const EnumValueDescriptor* pType = st_value_get_etype( pField, L);
				if (NULL == pType) {
					DF_LOG(LOG_ERROR, "Fail to find enum descriptor name:" << lua_tostring(L, -1));
				}
				else {
					pRef->AddEnum(msg, pField, pType);
				}
		}
		case FieldDescriptor::CPPTYPE_STRING:{
			size_t len = 0;
			const char* c = lua_tolstring(L, -1, &len);
			pRef->AddString(msg, pField, std::string(c, len));
			break;
		}
		default:{
				luaL_argerror(L, (2), " set not support type");
				DF_LOG(LOG_ERROR, " not support name:" << pField->name() << " type:" << pField->type_name());
				break;
		}
	}
}
//lua table to pb 
static  void st_table_to_pb(Message* msg, const FieldDescriptor* pField, lua_State* L)
{
	if ( NULL == msg ){
		//lua_pop(L, 1);
		DF_LOG(LOG_ERROR, " no message " );
		return ;
	}
	const int it = lua_gettop(L);
	if ( 1 == lua_isnil(L, it) ){
		DF_LOG(LOG_ERROR, " not table top " << it );
		lua_pop(L, 1);
		return;
	}
	lua_pushnil(L);

	const Reflection* pRef = msg->GetReflection();
	const Descriptor* pDsp = msg->GetDescriptor();
	while ( 0 != lua_next(L, it) ){
		if ( LUA_TNUMBER == lua_type(L, -2)){
			if ( NULL != pField && pField->is_repeated() ){
				st_value_to_pb_repeated( pRef , pField, msg, L);
			}
		}
		else{
			const std::string      fieldName = luaL_checkstring(L, -2);
			const FieldDescriptor* k_pfield  = pDsp->FindFieldByName(fieldName);
			if ( k_pfield ){
				if (k_pfield->is_repeated()){
					st_table_to_pb( msg, k_pfield, L);
				}
				else{
					st_value_to_pb( pRef, k_pfield, msg, L);
				}
			}
		}
		lua_pop(L, 1);
	}
}
//===============================================================
static int st_luapb_reg_path(lua_State *L)
{
	g_sourceTree.MapPath("", luaL_checkstring(L, 1) );
	DF_LOG(LOG_NORMAL, " reg proto path:" << luaL_checkstring(L, 1) );
	return 0; 
}
static int st_luapb_reg_file(lua_State *L)
{
	if (NULL == g_importer.Import( luaL_checkstring(L, 1) )){
		lua_pushboolean(L, 0);
		DF_LOG(LOG_ERROR, " import fail file:" << luaL_checkstring(L, 1) );
	}
	else{
		lua_pushboolean(L, 1);
		DF_LOG(LOG_NORMAL, " reg proto file:" << luaL_checkstring(L, 1));
	}
	return 1;
}
//编码包 
static int st_luapb_encode(lua_State *L)
{
	const char       *name       = luaL_checkstring(L, 1);
	const Descriptor *descriptor = g_importer.pool()->FindMessageTypeByName( name );
	if ( NULL == descriptor ){
		lua_pushnil( L );
		DF_LOG(LOG_ERROR, "get descriptor fail :" << name );
		return 1;
	}
	const Message *message = g_factory.GetPrototype( descriptor );
	if ( NULL == message ){
		lua_pushnil(L);
		DF_LOG(LOG_ERROR, "get GetPrototype fail :" << name );
		return 1;
	}
	internal::scoped_ptr<Message> k_pMsg( message->New() );
	st_table_to_pb( k_pMsg.get(), NULL, L );

	const std::string val = k_pMsg->SerializeAsString();
	lua_pushlstring(L, val.data(), val.size() );

	DF_LOG(LOG_NORMAL, "encode msg out:" << k_pMsg->ShortDebugString() );
	return  1; 
}

//解码 msg_name, buf 
static int st_luapb_decode( lua_State *L )
{
	size_t len(0);
	const char *name = luaL_checkstring (L, 1 );
	const char *buf  = luaL_checklstring(L, 2, &len);

	const Descriptor *pDescriptor = g_importer.pool()->FindMessageTypeByName( name );
	if ( NULL == pDescriptor ){
		lua_pushnil(L);
		DF_LOG(LOG_ERROR, "get descriptor fail :" << name);
		return 1;
	}
	const Message *msg = g_factory.GetPrototype( pDescriptor );
	if ( NULL == msg ){
		lua_pushnil(L);
		DF_LOG(LOG_ERROR, "get GetPrototype fail :" << name);
		return 1;
	}
	internal::scoped_ptr<Message> k_pMsg( msg->New() );
	if ( !k_pMsg->ParseFromArray( (const void*)buf, len) ){
		lua_pushnil(L);
		DF_LOG(LOG_ERROR, "get ParseFromArray fail :" << name );
		return 1;
	}
	DF_LOG(LOG_NORMAL, "decode msg :" << k_pMsg->ShortDebugString());
	return st_pb_to_table( *k_pMsg.get(), L );
}

////////////////////////////////////////////////////////////
//设置日志级别 
static int st_set_log(lua_State *L)
{
	LogFile::instance().m_log_level = luaL_checknumber(L, 1);
	DF_LOG(5, " set log " << LogFile::instance().m_log_level  );
	return 0; 
}
//luapb 
static const luaL_Reg  luapb_module[] =
{
	{ "set_log",       st_set_log        },
	{ "import",        st_luapb_reg_file },
	{ "encode",        st_luapb_encode   },
	{ "decode",        st_luapb_decode   },
	{ "register_file", st_luapb_reg_file },
	{ "register_path", st_luapb_reg_path },
	{ NULL, NULL }
};

extern "C" int luaopen_luapb(lua_State *L)
{
	g_sourceTree.MapPath("", "./");
	luaL_register(L, "pb", luapb_module);
	return 1;
}
