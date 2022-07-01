//===========================================================================
// Copyright (C) 2022 Intel Corporation
//
//
//
// SPDX-License-Identifier: MIT
//--------------------------------------------------------------------------

/**
 *
 * @file  I2C_AUX_Sample_App.cpp
 * @brief This file contains the I2C AUX Sample App & the 'main' function. Program execution begins and ends there.
 *
 */

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#define CTL_APIEXPORT // caller of control API DLL shall define this before
                      // including igcl_api.h
#include "igcl_api.h"
#include "GenericIGCLApp.h"

ctl_result_t GResult = CTL_RESULT_SUCCESS;

/***************************************************************
 * @brief TestI2CAUXAccess
 * Reference code to use I2CAuxAccess API
 * @param hDisplayOutput
 * @return ctl_result_t
 ***************************************************************/
ctl_result_t TestI2CAUXAccess(ctl_display_output_handle_t hDisplayOutput)
{
    ctl_result_t Result           = CTL_RESULT_SUCCESS;
    ctl_aux_access_args_t AUXArgs = { 0 }; // AUX Access WRITE
    ctl_i2c_access_args_t I2CArgs = { 0 }; // I2C Access
    AUXArgs.Size                  = sizeof(ctl_aux_access_args_t);
    AUXArgs.OpType                = CTL_OPERATION_TYPE_WRITE;
    AUXArgs.Address               = 0x103; // DPCD offset for TRAINING_LANE0_SET
    AUXArgs.DataSize              = 1;
    AUXArgs.Flags                 = CTL_AUX_FLAG_NATIVE_AUX; // CTL_AUX_FLAG_NATIVE_AUX for DPCD Access & CTL_AUX_FLAG_I2C_AUX for EDID access for DP/eDP displays.
    AUXArgs.Data[0]               = 0x01;

    Result = ctlAUXAccess(hDisplayOutput, &AUXArgs);

    if (CTL_RESULT_SUCCESS != Result)
    {
        printf("ctlAUXAccess for Native Aux DPCD WRITE returned failure code: 0x%X\n", Result);
        STORE_AND_RESET_ERROR(Result);
    }

    // AUX Access READ
    AUXArgs.Size     = sizeof(ctl_aux_access_args_t);
    AUXArgs.OpType   = CTL_OPERATION_TYPE_READ;
    AUXArgs.Address  = 0x103; // Address used for demonstration purpose
    AUXArgs.DataSize = 1;
    AUXArgs.Flags    = CTL_AUX_FLAG_NATIVE_AUX;

    EXIT_ON_MEM_ALLOC_FAILURE(hDisplayOutput, "hDisplayOutput");

    Result = ctlAUXAccess(hDisplayOutput, &AUXArgs);

    if (CTL_RESULT_SUCCESS != Result)
    {
        printf("ctlAUXAccess for Native Aux DPCD Read returned failure code: 0x%X\n", Result);
        STORE_AND_RESET_ERROR(Result);
    }
    else
    {
        // Print the data
        for (uint32_t j = 0; j < AUXArgs.DataSize; j++)
        {
            printf("Read data[%d] = : 0x%X\n", j, AUXArgs.Data[j]);
        }
    }

    // I2C WRITE : 82 01 10 AC at adddress 6E and subaddress 51
    // If we write these BYTEs ( 82 01 10  AC) to adddress 6E and
    // subaddress 51, it should update the current brightness to the 10th
    // byte at adddress 6E and subaddress 51. One can verify by changing
    // panel brightness from panel buttons and the writing to adddress 6E
    // and subaddress 51 ( 82 01 10  AC), and then reading 10th byte at
    // adddress 6E and subaddress 51. For Example : The following 11 byte
    // values should be shown by the I2C Read post I2C write. Values are
    // 6E 88 02 00 10 00 00 64 00 19 D9.  (If HDMI panel brightness is set
    // to 25%) 10th byte value is current  brightness value of the
    // panel.To confirm that value is correct or not, convert the Hex
    // value to Decimal.

    I2CArgs.Size     = sizeof(ctl_i2c_access_args_t);
    I2CArgs.OpType   = CTL_OPERATION_TYPE_WRITE;
    I2CArgs.Address  = 0x6E; // Address used for demonstration purpose
    I2CArgs.Offset   = 0x51; // Offset used for demonstration purpose
    I2CArgs.DataSize = 4;
    I2CArgs.Data[0]  = 0x82;
    I2CArgs.Data[1]  = 0x01;
    I2CArgs.Data[2]  = 0x10;
    I2CArgs.Data[3]  = 0xAC;

    EXIT_ON_MEM_ALLOC_FAILURE(hDisplayOutput, "hDisplayOutput");

    Result = ctlI2CAccess(hDisplayOutput, &I2CArgs);

    if (CTL_RESULT_SUCCESS != Result)
    {
        printf("ctlI2CAccess for I2C write returned failure code: 0x%X\n", Result);
        STORE_AND_RESET_ERROR(Result);
    }

    // I2C READ : 82 01 10 AC at adddress 6E and subaddress 51
    I2CArgs.Size     = sizeof(ctl_i2c_access_args_t);
    I2CArgs.OpType   = CTL_OPERATION_TYPE_READ;
    I2CArgs.Address  = 0x6E; // Address used for demonstration purpose
    I2CArgs.Offset   = 0x51; // Offset used for demonstration purpose
    I2CArgs.DataSize = 11;
    // I2CArgs.Flags = CTL_I2C_FLAG_ATOMICI2C;  // Need to set this to do Atomic I2C call

    Result = ctlI2CAccess(hDisplayOutput, &I2CArgs);
    LOG_AND_EXIT_ON_ERROR(Result, "ctlI2CAccess for I2C read");

    //  Print the data
    for (uint32_t j = 0; j < I2CArgs.DataSize; j++)
    {
        printf("Read data[%d] = : 0x%X\n", j, I2CArgs.Data[j]);
    }

Exit:

    return Result;
}

/***************************************************************
 * @brief EnumerateDisplayHandles
 * Only for demonstration purpose, API is called for each of the display output handle in below snippet.
 * User has to filter through the available display output handle and has to call the API with particular display output handle.
 * @param hDisplayOutput, DisplayCount
 * @return ctl_result_t
 ***************************************************************/
ctl_result_t EnumerateDisplayHandles(ctl_display_output_handle_t *hDisplayOutput, uint32_t DisplayCount)
{
    ctl_result_t Result = CTL_RESULT_SUCCESS;

    for (uint32_t DisplayIndex = 0; DisplayIndex < DisplayCount; DisplayIndex++)
    {
        ctl_display_properties_t DisplayProperties = { 0 };
        DisplayProperties.Size                     = sizeof(ctl_display_properties_t);

        Result = ctlGetDisplayProperties(hDisplayOutput[DisplayIndex], &DisplayProperties);
        LOG_AND_EXIT_ON_ERROR(Result, "ctlGetDisplayProperties");

        bool IsDisplayAttached = (0 != (DisplayProperties.DisplayConfigFlags & CTL_DISPLAY_CONFIG_FLAG_DISPLAY_ATTACHED));

        if (FALSE == IsDisplayAttached)
        {
            printf("Display %d is not attached, skipping the call for this display\n", DisplayIndex);
            continue;
        }

        Result = TestI2CAUXAccess(hDisplayOutput[DisplayIndex]);
        STORE_AND_RESET_ERROR(Result);
    }

Exit:
    return Result;
}

/***************************************************************
 * @brief EnumerateTargetDisplays
 * Enumerates all the possible target display's for the adapters
 * @param hDisplayOutput, AdapterCount, hDevices
 * @return ctl_result_t
 ***************************************************************/
ctl_result_t EnumerateTargetDisplays(uint32_t AdapterCount, ctl_device_adapter_handle_t *hDevices)
{
    ctl_display_output_handle_t *hDisplayOutput = NULL;
    ctl_result_t Result                         = CTL_RESULT_SUCCESS;
    uint32_t DisplayCount                       = 0;

    for (uint32_t AdapterIndex = 0; AdapterIndex < AdapterCount; AdapterIndex++)
    {
        // enumerate all the possible target display's for the adapters
        // first step is to get the count
        DisplayCount = 0;

        Result = ctlEnumerateDisplayOutputs(hDevices[AdapterIndex], &DisplayCount, hDisplayOutput);

        if (CTL_RESULT_SUCCESS != Result)
        {
            printf("ctlEnumerateDisplayOutputs returned failure code: 0x%X\n", Result);
            STORE_AND_RESET_ERROR(Result);
            continue;
        }
        else if (DisplayCount <= 0)
        {
            printf("Invalid Display Count. skipping display enumration for adapter:%d\n", AdapterIndex);
            continue;
        }

        hDisplayOutput = (ctl_display_output_handle_t *)malloc(sizeof(ctl_display_output_handle_t) * DisplayCount);
        EXIT_ON_MEM_ALLOC_FAILURE(hDisplayOutput, "hDisplayOutput");

        Result = ctlEnumerateDisplayOutputs(hDevices[AdapterIndex], &DisplayCount, hDisplayOutput);

        if (CTL_RESULT_SUCCESS != Result)
        {
            printf("ctlEnumerateDisplayOutputs returned failure code: 0x%X\n", Result);
            STORE_AND_RESET_ERROR(Result);
        }

        // Only for demonstration purpose, API is called for each of the display output handle in below snippet.
        // User has to filter through the available display output handle and has to call the API with particular display output handle.
        Result = EnumerateDisplayHandles(hDisplayOutput, DisplayCount);

        if (CTL_RESULT_SUCCESS != Result)
        {
            printf("EnumerateDisplayHandles returned failure code: 0x%X\n", Result);
        }

        CTL_FREE_MEM(hDisplayOutput);
    }

Exit:

    CTL_FREE_MEM(hDisplayOutput);
    return Result;
}

/***************************************************************
 * @brief Main Function which calls the Sample I2CAuxAccess API
 * @param
 * @return int
 ***************************************************************/
int main()
{
    ctl_result_t Result                   = CTL_RESULT_SUCCESS;
    ctl_device_adapter_handle_t *hDevices = NULL;
    // Get a handle to the DLL module.
    uint32_t AdapterCount = 0;
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    ctl_init_args_t CtlInitArgs;
    ctl_api_handle_t hAPIHandle;

    ZeroMemory(&CtlInitArgs, sizeof(ctl_init_args_t));

    CtlInitArgs.AppVersion = CTL_MAKE_VERSION(CTL_IMPL_MAJOR_VERSION, CTL_IMPL_MINOR_VERSION);
    CtlInitArgs.flags      = 0;
    CtlInitArgs.Size       = sizeof(CtlInitArgs);
    CtlInitArgs.Version    = 0;

    Result = ctlInit(&CtlInitArgs, &hAPIHandle);
    LOG_AND_EXIT_ON_ERROR(Result, "ctlInit");

    // Initialization successful
    // Get the list of Intel Adapters
    Result = ctlEnumerateDevices(hAPIHandle, &AdapterCount, hDevices);
    LOG_AND_EXIT_ON_ERROR(Result, "ctlEnumerateDevices");

    hDevices = (ctl_device_adapter_handle_t *)malloc(sizeof(ctl_device_adapter_handle_t) * AdapterCount);
    EXIT_ON_MEM_ALLOC_FAILURE(hDevices, "hDevices");

    Result = ctlEnumerateDevices(hAPIHandle, &AdapterCount, hDevices);
    LOG_AND_EXIT_ON_ERROR(Result, "ctlEnumerateDevices");

    Result = EnumerateTargetDisplays(AdapterCount, hDevices);

    if (CTL_RESULT_SUCCESS != Result)
    {
        printf("EnumerateTargetDisplays returned failure code: 0x%X\n", Result);
        STORE_AND_RESET_ERROR(Result);
    }

Exit:

    ctlClose(hAPIHandle);
    CTL_FREE_MEM(hDevices);
    printf("Overrall test result is 0x%X\n", GResult);
    return GResult;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started:
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add
//   Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project
//   and select the .sln file
