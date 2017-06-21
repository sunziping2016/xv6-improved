/** @file
 * @brief Xv6 system call functions
 * @author Qifan Lu
 * @date April 10, 2016
 * @version 1.0.0
 */
#pragma once

//[ Functions ]
/**
 * Socket-releated system call
 *
 * @param fd Socket file descriptor
 * @param oper Socket system call operation
 * @param _param Extra parameters
 */
extern int xv6_sys_sockcall(int fd, int oper, void* _param);
