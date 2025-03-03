#include "System/VisitStageData.h"

VisitStageData::VisitStageData() {}

void VisitStageData::init() {}

void VisitStageData::checkAlreadyVisit(const char* stageName) const {}

void VisitStageData::visit(const char* stageName) {}

void VisitStageData::write(al::ByamlWriter* writer) {}

void VisitStageData::read(const al::ByamlIter& iter) {}
