/**
 * @file printer.hpp
 * @author Jayson Sho Toma
 * @brief Interface class which requires to "dump" the class information on the standard output to the concrete classes.
 * @version 0.1
 * @date 2022-05-03
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

 /**
  * @class IPrinter
  * @brief The interface class which requires to "dump" the class information on the standard to the concrete classes.
  */
class IPrinter {
public:
    /**
     * @brief Destroy the IPrinter object
     *
     */
    virtual ~IPrinter() {}
    /**
     * @brief outputs class information on the standard output.
     *
     */
    virtual void dump() const = 0;
};