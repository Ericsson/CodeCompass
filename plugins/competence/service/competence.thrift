include "project/common.thrift"

namespace cpp cc.service.competence

service CompetenceService
{
    string setCompetenceRatio(1:common.FileId fileId, 2:i32 ratio)
}