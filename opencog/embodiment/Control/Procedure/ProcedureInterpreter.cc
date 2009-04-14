/*
 * opencog/embodiment/Control/Procedure/ProcedureInterpreter.cc
 *
 * Copyright (C) 2007-2008 TO_COMPLETE
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License v3 as
 * published by the Free Software Foundation and including the exceptions
 * at http://opencog.org/wiki/Licenses
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program; if not, write to:
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include <opencog/util/mt19937ar.h>
#include <opencog/util/exceptions.h>

#include "ProcedureInterpreter.h"
#include "ComboProcedure.h"
#include "ComboSelectProcedure.h"
#include "RunningProcedureId.h"
#include <opencog/embodiment/Control/MessagingSystem/NetworkElement.h>

namespace Procedure
{
using namespace boost;

void ProcedureInterpreter::run(MessagingSystem::NetworkElement *ne)
{

    // If a separate Agent is used for comboInterpreter, just comment the
    // next line :
    comboInterpreter->run(ne);

    Set toBeRemoved;
    // Runs each pending RunningBuildInProcedure and checks status of any procedure.
    for (Map::iterator it = _map.begin(); it != _map.end(); it++) {
        RunningBuiltInProcedure* rbp = get<RunningBuiltInProcedure>(&(it->second));
        if (rbp) {
            rbp->run();
            if (rbp->isFinished()) {
                toBeRemoved.insert(it->first);
                if (rbp->isFailed()) {
                    _failed.insert(it->first);
                } else {
                    _resultMap[it->first] = rbp->getResult();
                }
            }
        } else {
            RunningProcedureId rcpID = get<RunningProcedureId>(it->second);
            if (comboInterpreter->isFinished(rcpID)) {
                toBeRemoved.insert(it->first);
                if (comboInterpreter->isFailed(rcpID)) {
                    _failed.insert(it->first);
                } else {
                    _resultMap[it->first] = comboInterpreter->getResult(rcpID);
                    _unifierResultMap[it->first] = comboInterpreter->getUnifierResult(rcpID);
                }
            }
        }

    }
    for (Set::iterator it = toBeRemoved.begin(); it != toBeRemoved.end(); it++) {
        _map.erase(*it);
    }
}

ProcedureInterpreter::ProcedureInterpreter(PerceptionActionInterface::PAI& p) : _pai(&p)
{
    //initialize the random generator
    unsigned long rand_seed;
    if (opencog::config().get_bool("AUTOMATED_SYSTEM_TESTS")) {
        rand_seed = 0;
    } else {
        rand_seed = time(NULL);
    }
    rng = new opencog::MT19937RandGen(rand_seed);
    logger().log(opencog::Logger::INFO, "Created random number generator (%p) for ComboInterpreter with seed %lu", rng, rand_seed);
    comboInterpreter = new ComboInterpreter(*_pai, *rng);
    comboSelectInterpreter = new ComboSelectInterpreter(*_pai, *rng);
    _next = 0;
}

ProcedureInterpreter::~ProcedureInterpreter()
{
    delete comboInterpreter;
    delete comboSelectInterpreter;
    delete rng;
}

RunningProcedureID ProcedureInterpreter::runProcedure(const GeneralProcedure& p, const std::vector<combo::vertex>& arguments)
{

    logger().log(opencog::Logger::INFO,
                 "ProcedureInterpreter - runProcedure(%s)", p.getName().c_str());

    if (p.getType() == COMBO) {
        logger().log(opencog::Logger::DEBUG,
                     "ProcedureInterpreter - Running a combo procedure.");
        RunningProcedureId rcpID = comboInterpreter->runProcedure(((const ComboProcedure&) p).getComboTree(), arguments);
        _map.insert(make_pair(++_next, rcpID));
    } else if (p.getType() == COMBO_SELECT) {
        logger().log(opencog::Logger::DEBUG,
                     "ProcedureInterpreter - Running a combo select procedure.");
        const ComboSelectProcedure& procedure = ((const ComboSelectProcedure&) p);
        RunningProcedureId rcpID =
            comboSelectInterpreter->runProcedure(procedure.getFirstScript(),
                                                 procedure.getSecondScript(),
                                                 arguments);
        _map.insert(make_pair(++_next, rcpID));

    } else if (p.getType() == BUILT_IN) {
        logger().log(opencog::Logger::DEBUG,
                     "ProcedureInterpreter - Running a builtin procedure.");
        RunningBuiltInProcedure rbp = RunningBuiltInProcedure(*_pai, (const BuiltInProcedure&) p, arguments);
        // For now, runs built-in procedure immediately, since they are atomic
        // and caller may want to check for failure or get its result synchronously.
        rbp.run();
        _map.insert(make_pair(++_next, rbp));
    } else {
        opencog::cassert(TRACE_INFO, false, "ProcedureInterpreter -  unknown procedure type");
    }
    return _next;
}

RunningProcedureID ProcedureInterpreter::runProcedure(const GeneralProcedure& p, const std::vector<combo::vertex>& arguments, combo::variable_unifier& vu)
{
    logger().log(opencog::Logger::INFO,
                 "ProcedureInterpreter - runProcedure(%s)", p.getName().c_str());

    if (p.getType() == COMBO) {
        logger().log(opencog::Logger::DEBUG,
                     "ProcedureInterpreter - Running a combo procedure.");
        RunningProcedureId rcpID = comboInterpreter->runProcedure(((const ComboProcedure&) p).getComboTree(), arguments, vu);
        _map.insert(make_pair(++_next, rcpID));

    } else if (p.getType() == COMBO_SELECT) {
        logger().log(opencog::Logger::DEBUG,
                     "ProcedureInterpreter - Running a combo select procedure.");
        const ComboSelectProcedure& procedure = ((const ComboSelectProcedure&) p);
        RunningProcedureId rcpID = comboSelectInterpreter->runProcedure(procedure.getFirstScript(),
                                   procedure.getSecondScript(),
                                   arguments, vu);
        _map.insert(make_pair(++_next, rcpID));

    } else {
        opencog::cassert(TRACE_INFO, false, "ProcedureInterpreter - Only combo procedures accept variable unifier parameters.");
    }
    return _next;
}

bool ProcedureInterpreter::isFinished(RunningProcedureID id) const
{
    logger().log(opencog::Logger::FINE, "ProcedureInterpreter - isFinished(%lu).", id);
    bool result = true;
    Map::const_iterator it = _map.find(id);
    if (it != _map.end()) {
        RunningProcedure rp = it->second;
        RunningProcedureId* rpId;
        RunningBuiltInProcedure* rbp;
        if ((rbp = boost::get<RunningBuiltInProcedure>(&rp))) {
            result = rbp->isFinished();
        } else if ((rpId = boost::get<RunningProcedureId>(&rp))) {
            if (rpId->getType() == COMBO) {
                result = comboInterpreter->isFinished(*rpId);
            } else if (rpId->getType() == COMBO_SELECT) {
                result = comboSelectInterpreter->isFinished(*rpId);
            }
        }
    }
    logger().log(opencog::Logger::DEBUG, "ProcedureInterpreter - isFinished(%lu)? Result: %d.",
                 id, result);
    return result;
}

bool ProcedureInterpreter::isFailed(RunningProcedureID id) const
{
    logger().log(opencog::Logger::FINE, "ProcedureInterpreter - isFailed(%lu).", id);
    bool result = false;
    Map::const_iterator it = _map.find(id);
    if (it != _map.end()) {
        const RunningProcedureId* rpId;

        if ((rpId = boost::get<RunningProcedureId>(&(it->second)))) {
            if (rpId->getType() == COMBO) {
                result = comboInterpreter->isFailed(*rpId);
            } else if (rpId->getType() == COMBO_SELECT) {
                result = comboSelectInterpreter->isFailed(*rpId);
            }

        } else {
            result = boost::get<RunningBuiltInProcedure>(it->second).isFailed();
        }
    } else {
        result = (_failed.find(id) != _failed.end());
    }
    logger().log(opencog::Logger::DEBUG, "ProcedureInterpreter - isFailed(%lu)? Result: %d.",
                 id, result);
    return result;
}

combo::vertex ProcedureInterpreter::getResult(RunningProcedureID id)
{
    logger().log(opencog::Logger::FINE, "ProcedureInterpreter - getResult(%lu).", id);
    opencog::cassert(TRACE_INFO, isFinished(id), "ProcedureInterpreter - Procedure '%d' not finished.", id);
    opencog::cassert(TRACE_INFO, !isFailed(id), "ProcedureInterpreter - Procedure '%d' failed.", id);

    combo::vertex result = combo::id::action_success;
    Map::const_iterator it = _map.find(id);

    if (it != _map.end()) {
        RunningBuiltInProcedure* rbp;
        RunningProcedureId* rpId;
        RunningProcedure rp = it->second;

        if ((rbp = boost::get<RunningBuiltInProcedure>(&rp))) {
            result = rbp->getResult();

        } else if ((rpId = boost::get<RunningProcedureId>(&rp))) {
            if (rpId->getType() == COMBO) {
                result = comboInterpreter->getResult(*rpId);
            } else if (rpId->getType() == COMBO_SELECT) {
                result = comboSelectInterpreter->getResult(*rpId);
            }
        }

    } else {
        ResultMap::iterator it = _resultMap.find(id);
        opencog::cassert(TRACE_INFO, it != _resultMap.end(),
                         "ProcedureInterpreter - Cannot find result for procedure '%d'.", id);
        result = it->second;
    }
    return result;
}

combo::variable_unifier& ProcedureInterpreter::getUnifierResult(RunningProcedureID id)
{
    opencog::cassert(TRACE_INFO, isFinished(id), "ProcedureInterpreter - Procedure '%d' not finished.", id);
    opencog::cassert(TRACE_INFO, !isFailed(id), "ProcedureInterpreter - Procedure '%d' failed.", id);
    UnifierResultMap::iterator it = _unifierResultMap.find(id);
    opencog::cassert(TRACE_INFO, it != _unifierResultMap.end(),
                     "ProcedureInterpreter - Cannot find unifier result for procedure '%d'.", id);
    return it->second;
}

void ProcedureInterpreter::stopProcedure(RunningProcedureID id)
{
    logger().log(opencog::Logger::FINE, "ProcedureInterpreter - stopProcedure(%lu).", id);
    Map::iterator it = _map.find(id);
    if (it != _map.end()) {
        RunningProcedureId* rpId = boost::get<RunningProcedureId>(&(it->second));
        if (rpId->getType() == COMBO) {
            comboInterpreter->stopProcedure(*rpId);
        } else if (rpId->getType() == COMBO_SELECT) {
            comboSelectInterpreter->stopProcedure(*rpId);
        }
        _map.erase(it);
    }
    Set::iterator failed_it = _failed.find(id);
    if (failed_it != _failed.end()) {
        _failed.erase(failed_it);
    }
    ResultMap::iterator result_it = _resultMap.find(id);
    if (result_it != _resultMap.end()) {
        _resultMap.erase(result_it);
    }
}

ComboInterpreter& ProcedureInterpreter::getComboInterpreter() const
{
    return *comboInterpreter;
}

ComboSelectInterpreter& ProcedureInterpreter::getComboSelectInterpreter() const
{
    return *comboSelectInterpreter;
}


} //~namespace Procedure


