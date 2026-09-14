#ifndef QS_NULLABLETYPE_HPP
#define QS_NULLABLETYPE_HPP
#include "ScriptStructType.hpp"
#include "ScriptType.hpp"

class NullableType: public ScriptType {
  ScriptStructType* const m_targetType;

  public:
    explicit NullableType(ScriptStructType* targetType);

    ScriptStructType* getTargetType();
};

#endif //QS_NULLABLETYPE_HPP
