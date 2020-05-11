#pragma once

#define INIT R"###(

#define PLUGIN_INIT_METHOD \
virtual void init() \
{ \
${INSERTS} \
}
)###"
