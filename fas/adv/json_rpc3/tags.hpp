//
// Author: Vladimir Migashko <migashko@faslib.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAS_ADV_JSON_RPC3_TAGS_HPP
#define FAS_ADV_JSON_RPC3_TAGS_HPP

namespace fas{ namespace adv{ namespace json_rpc3{

struct _input_{};
struct _output_{};

struct _writer_{};

// общий буфер для всех адвайсов json для сериализации (чтоб память сэкономить немного ))) )
struct _outgoing_data_{};

struct _invoke_{};
struct _begin_invoke_{}; // уведомление - начало разбора пакета запросов
struct _end_invoke_{};   // уведомление - окончание разбора пакета запросов
struct _gmethod_{};

struct _unknown_method_{};
struct _invalid_json_{};
struct _invalid_id_{};
struct _lost_result_{};


/// биндер для команд
struct _binder_{};

}}}

#endif

