/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#include "EditorDefs.h"
#include <AzTest/AzTest.h>
#include "Util/PathUtil.h"
#include <CrySystemBus.h>

TEST(PathUtil, GamePathToFullPath_DoesNotBufferOverflow)
{
    // There are no test assertions in this test because the purpose is just to verify that the test runs without crashing
    QString pngExtension(".png");

    // Create a string of lenth AZ_MAX_PATH_LEN that ends in .png
    QString longStringMaxPath(AZ_MAX_PATH_LEN, 'x');
    longStringMaxPath.replace(longStringMaxPath.length() - pngExtension.length(), longStringMaxPath.length(), pngExtension);
    Path::GamePathToFullPath(longStringMaxPath);

    QString longStringMaxPathPlusOne(AZ_MAX_PATH_LEN + 1, 'x');
    longStringMaxPathPlusOne.replace(longStringMaxPathPlusOne.length() - pngExtension.length(), longStringMaxPathPlusOne.length(), pngExtension);
    Path::GamePathToFullPath(longStringMaxPathPlusOne);
}
