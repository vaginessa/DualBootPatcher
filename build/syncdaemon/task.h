/*
 * Copyright (C) 2014  Xiao-Long Chen <chenxiaolong@cxl.epac.to>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TASK_H
#define TASK_H

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>

// Defer execution of a single task. The task will only be run once no matter
// how many times it's asked to be executed if the timer hasn't expired yet.
class SingleDelayedTask {
public:
    // Return false if thread should be killed
    bool wait() {
        std::unique_lock<std::mutex> mlock(mutex_);
        while (!can_run && !should_kill) {
            cond_.wait(mlock);
        }

        for (int i = 0; i < delay; i++) {
            if (should_kill) {
                should_kill = false;
                return false;
            } else {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }

        can_run = false;
        return true;
    }

    void execute() {
        if (!can_run) {
            std::cout << "Task execution will delayed by "
                    << delay << " seconds" << std::endl;

            std::unique_lock<std::mutex> mlock(mutex_);
            can_run = true;
            mlock.unlock();
            cond_.notify_one();
        } else {
            std::cout << "Task already waiting to be executed" << std::endl;
        }
    }

    void kill() {
        if (!should_kill) {
            std::cout << "Killing thread..." << std::endl;
            should_kill = true;
            cond_.notify_one();
        }
    }

    SingleDelayedTask(unsigned int seconds) : delay(seconds) {
    }

    SingleDelayedTask() = default;
    SingleDelayedTask(const SingleDelayedTask&) = delete;
    SingleDelayedTask& operator=(const SingleDelayedTask&) = delete;

private:
    unsigned int delay;
    bool can_run;
    bool should_kill;
    std::mutex mutex_;
    std::condition_variable cond_;
};

#endif //TASK_H