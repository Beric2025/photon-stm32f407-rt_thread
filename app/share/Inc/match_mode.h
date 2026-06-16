/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */
#ifndef __MATCH_MODE_H
#define __MATCH_MODE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

#define ROLLING_WINDOW_SIZE 10

/**
 * @brief  Rolling average filter structure
 */
typedef struct {
    float rollingData[ROLLING_WINDOW_SIZE];
    uint8_t dataCount;
    uint8_t isFull;
} Rolling_Avg_Filter_T;

/**
 * @brief:  initialize rolling average filter
 * @filter:  filter instance pointer
 */
void rolling_averageg_init(Rolling_Avg_Filter_T *filter);

/**
 * @brief:  apply rolling average filter to float data
 * @filter:  filter instance pointer
 * @data:     new sample value
 *
 * Return: filtered value
 */
float rolling_averageg_filter(Rolling_Avg_Filter_T *filter, float data);

/**
 * @brief:  apply rolling average filter to int32 data
 * @filter:  filter instance pointer
 * @data:     new sample value
 *
 * Return: filtered value
 */
int32_t rolling_averageg_filter_int(Rolling_Avg_Filter_T *filter, int32_t data);

#ifdef __cplusplus
}
#endif

#endif /* __MATCH_MODE_H */
