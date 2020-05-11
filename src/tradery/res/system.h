#pragma once

#define SYSTEM R"###(
/*********************************************/
#undef SYSTEM_ID
#undef SYSTEM_NAME
#undef SYSTEM_DESCRIPTION

#define SYSTEM_ID "${SYSTEM_UUID}"

class ${SYSTEM_CLASS_NAME} : public BarSystem<${SYSTEM_CLASS_NAME} > {
 public:
  ${SYSTEM_CLASS_NAME} (const std::vector<std::string>* params = 0);

 private:

${SYSTEM_CODE}
};

#if !defined( SYSTEM_NAME )
  #define SYSTEM_NAME "<no name given>"
#endif

#if !defined( SYSTEM_DESCRIPTION )
  #define SYSTEM_DESCRIPTION "<no description given>"
#endif

inline ${SYSTEM_CLASS_NAME}::${SYSTEM_CLASS_NAME} (const std::vector<std::string>* params)
      : BarSystem<${SYSTEM_CLASS_NAME} >(
            Info("${SYSTEM_UUID}", SYSTEM_NAME, SYSTEM_DESCRIPTION),
            "${SYSTEM_DB_ID}") {}


#pragma message("#systemName=" SYSTEM_NAME)
#pragma message("#className=${SYSTEM_CLASS_NAME}")

)###"