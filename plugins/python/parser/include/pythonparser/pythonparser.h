#ifndef CODECOMPASS_PYTHONPARSER_H
#define CODECOMPASS_PYTHONPARSER_H


#include <parser/abstractparser.h>
#include <parser/parsercontext.h>

namespace cc
{
namespace parser
{

class PythonParser : public AbstractParser {
public:
    PythonParser(ParserContext &ctx_);

    virtual ~PythonParser();

    virtual void markModifiedFiles() override;

    virtual bool cleanupDatabase() override;

    virtual bool parse() override;
};

} // parser
} // cc

#endif //CODECOMPASS_PYTHONPARSER_H
