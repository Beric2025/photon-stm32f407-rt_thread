/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * Rolling average filter module
 */

#include "match_mode.h"
#include <math.h>
#include <string.h>


void rolling_averageg_init(Rolling_Avg_Filter_T *filter)
{
    if(filter != NULL)
    {
        memset(filter, 0, sizeof(Rolling_Avg_Filter_T));
        for(int i = 0; i < ROLLING_WINDOW_SIZE; i++)
        {
            filter->rollingData[i] = 0.0f;
        }
        filter->dataCount = 0;
        filter->isFull = 0;
    }
}


float rolling_averageg_filter(Rolling_Avg_Filter_T *filter, float data)
{
    if(filter == NULL)
    {
        return 0.0f;
    }

    /* Push new data into the rolling window */
    if(filter->isFull)
    {
        /* Window full: shift data forward and replace oldest entry */
        for(int i = 0; i < ROLLING_WINDOW_SIZE - 1; i++)
        {
            filter->rollingData[i] = filter->rollingData[i + 1];
        }
        filter->rollingData[ROLLING_WINDOW_SIZE - 1] = data;
    }
    else
    {
        /* Window not full: append data directly */
        filter->rollingData[filter->dataCount] = data;
        filter->dataCount++;
        if(filter->dataCount == ROLLING_WINDOW_SIZE)
        {
            filter->isFull = 1;
        }
    }

    /* Determine current number of valid data points */
    uint8_t current_size = filter->isFull ? ROLLING_WINDOW_SIZE : filter->dataCount;

    if(current_size <= 2)
    {
        /* Too few samples for min/max discard; return simple average */
        float sum = 0;
        for(int i = 0; i < current_size; i++)
        {
            sum += filter->rollingData[i];
        }
        return sum / current_size;
    }

    /* Locate max and min values and their indices */
    float max_val = filter->rollingData[0], min_val = filter->rollingData[0];
    int max_idx = 0, min_idx = 0;

    for(int i = 1; i < current_size; i++)
    {
        if(filter->rollingData[i] > max_val)
        {
            max_val = filter->rollingData[i];
            max_idx = i;
        }
        if(filter->rollingData[i] < min_val)
        {
            min_val = filter->rollingData[i];
            min_idx = i;
        }
    }

    /* If all values are identical, return the simple average */
    if(max_idx == min_idx)
    {
        float sum = 0;
        for(int i = 0; i < current_size; i++)
        {
            sum += filter->rollingData[i];
        }
        return sum / current_size;
    }

    /* Sum all values except max and min */
    float sum = 0;
    for(int i = 0; i < current_size; i++)
    {
        if(i != max_idx && i != min_idx)
        {
            sum += filter->rollingData[i];
        }
    }

    /* Return trimmed mean */
    return sum / (current_size - 2);
}


int32_t rolling_averageg_filter_int(Rolling_Avg_Filter_T *filter, int32_t data)
{
    if(filter == NULL)
    {
        return 0;
    }

    /* Delegate to float version, then round to nearest integer */
    float result = rolling_averageg_filter(filter, (float)data);
    return (int32_t)(result + 0.5f);
}
