#ifndef ARKHelper_h
#define ARKHelper_h

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <ARKit/ARKit.h>

/**
 Enum representing the WebXR authorization status
 
 - WebXRAuthorizationStateNotDetermined: The user didn't say anything about the world sensing
 - WebXRAuthorizationStateDenied: The user denied sending world sensing data
 - WebXRAuthorizationStateMinimal: The user allowed sending wold sensing data
 - WebXRAuthorizationStateLite: The user allowed sending wold sensing data
 - WebXRAuthorizationStateWorldSensing: The user allowed sending wold sensing data
 - WebXRAuthorizationStateVideoCameraAccess: The user allowed access to the video camera and sending wold sensing data
 
 */
typedef NS_ENUM(NSUInteger, WebXRAuthorizationState)
{
    WebXRAuthorizationStateNotDetermined,
    WebXRAuthorizationStateDenied,
    WebXRAuthorizationStateMinimal,
    WebXRAuthorizationStateLite,
    WebXRAuthorizationStateWorldSensing,
    WebXRAuthorizationStateVideoCameraAccess
};

/**
 An enum representing the state of the app UI at a given time
 
 - ShowNothing: Shows the warning labels
 - ShowDebug: Shows the helper and build label, and the AR debug info
 - ShowURL: Shows the URL Bar
 - ShowURLDebug: Shows the URL Bar and the AR debug info
 */
// Tony (2/12/19): In making the ShowMode enum more descriptive, there's temporary
//      awkwardness & bad style when using the enum in Swift (.URL & .urlDebug).
//      This will be resolved when fully converted to Swift.
typedef NS_ENUM(NSUInteger, ShowMode)
{
    ShowNothing,
    ShowDebug,
    ShowURL,
    ShowURLDebug
};

/**
 Show options. This option set is built from the AR Request dictionary received on initAR
 
 - None: Shows nothing
 - Browser: Shows in browser mode
 - ARWarnings: Shows warnings reported by ARKit
 - ARFocus: Shows a focus node
 - ARObject: Shows AR objects
 - Debug: Not used
 - ARPlanes: Shows AR planes
 - ARPoints: Shows AR feature points
 - ARStatistics: Shows AR Statistics
 - BuildNumber: Shows the app build number
 - Full: Shows everything
 */
typedef NS_OPTIONS(NSUInteger, ShowOptions)
{
    None         = 0,
    Browser      = (1 << 0),
    ARWarnings   = (1 << 1),
    ARFocus      = (1 << 2),
    ARObject     = (1 << 3),
    Debug        = (1 << 4),
    ARPlanes     = (1 << 5),
    ARPoints     = (1 << 6),
    ARStatistics = (1 << 7),
    BuildNumber  = (1 << 8),
    Full         = NSUIntegerMax
};

static inline NSArray * arrayFromMatrix3x3(matrix_float3x3  matrix)
{
    return @[@(matrix.columns[0][0]),
             @(matrix.columns[0][1]),
             @(matrix.columns[0][2]),
             @(matrix.columns[1][0]),
             @(matrix.columns[1][1]),
             @(matrix.columns[1][2]),
             @(matrix.columns[2][0]),
             @(matrix.columns[2][1]),
             @(matrix.columns[2][2])];
}

static inline NSArray * arrayFromMatrix4x4(matrix_float4x4  matrix)
{
    return @[@(matrix.columns[0][0]),
             @(matrix.columns[0][1]),
             @(matrix.columns[0][2]),
             @(matrix.columns[0][3]),
             @(matrix.columns[1][0]),
             @(matrix.columns[1][1]),
             @(matrix.columns[1][2]),
             @(matrix.columns[1][3]),
             @(matrix.columns[2][0]),
             @(matrix.columns[2][1]),
             @(matrix.columns[2][2]),
             @(matrix.columns[2][3]),
             @(matrix.columns[3][0]),
             @(matrix.columns[3][1]),
             @(matrix.columns[3][2]),
             @(matrix.columns[3][3])];
}

static inline matrix_float4x4 matrixFromArray(NSArray *arr)
{
    matrix_float4x4 matrix;
    
    matrix.columns[0][0] = [arr[0] floatValue];
    matrix.columns[0][1] = [arr[1] floatValue];
    matrix.columns[0][2] = [arr[2] floatValue];
    matrix.columns[0][3] = [arr[3] floatValue];
    matrix.columns[1][0] = [arr[4] floatValue];
    matrix.columns[1][1] = [arr[5] floatValue];
    matrix.columns[1][2] = [arr[6] floatValue];
    matrix.columns[1][3] = [arr[7] floatValue];
    matrix.columns[2][0] = [arr[8] floatValue];
    matrix.columns[2][1] = [arr[9] floatValue];
    matrix.columns[2][2] = [arr[10] floatValue];
    matrix.columns[2][3] = [arr[11] floatValue];
    matrix.columns[3][0] = [arr[12] floatValue];
    matrix.columns[3][1] = [arr[13] floatValue];
    matrix.columns[3][2] = [arr[14] floatValue];
    matrix.columns[3][3] = [arr[15] floatValue];
    
    return matrix;
}

static inline NSDictionary * dictFromVector3(vector_float3 vector)
{
    return @{@"x" : @(vector.x), @"y" : @(vector.y), @"z" : @(vector.z)};
}

static inline NSDictionary * dictFromMatrix4x4(matrix_float4x4  matrix)
{
    NSMutableDictionary *dict = [NSMutableDictionary dictionaryWithCapacity:16];
    
    dict[@"00"] = @(matrix.columns[0][0]);
    dict[@"01"] = @(matrix.columns[0][1]);
    dict[@"02"] = @(matrix.columns[0][2]);
    dict[@"03"] = @(matrix.columns[0][3]);
    dict[@"10"] = @(matrix.columns[1][0]);
    dict[@"11"] = @(matrix.columns[1][1]);
    dict[@"12"] = @(matrix.columns[1][2]);
    dict[@"13"] = @(matrix.columns[1][3]);
    dict[@"20"] = @(matrix.columns[2][0]);
    dict[@"21"] = @(matrix.columns[2][1]);
    dict[@"22"] = @(matrix.columns[2][2]);
    dict[@"23"] = @(matrix.columns[2][3]);
    dict[@"30"] = @(matrix.columns[3][0]);
    dict[@"31"] = @(matrix.columns[3][1]);
    dict[@"32"] = @(matrix.columns[3][2]);
    dict[@"33"] = @(matrix.columns[3][3]);
    
    return [dict copy];
}

static inline matrix_float4x4 matrixFromKeyedArrayDictionary(NSDictionary *dict);

static inline matrix_float4x4 matrixFromDictionary(NSDictionary *dict)
{
    if (dict[@"33"] == nil)
    {
        return matrixFromKeyedArrayDictionary(dict);
    }
    
    matrix_float4x4 matrix;
    NSString *stringValue = dict[@"00"] ? dict[@"00"] : dict[@"0"];
    matrix.columns[0][0] = [stringValue floatValue];
    
    stringValue = dict[@"01"] ? dict[@"01"] : dict[@"1"];
    matrix.columns[0][1] = [stringValue floatValue];
    
    stringValue = dict[@"02"] ? dict[@"02"] : dict[@"2"];
    matrix.columns[0][2] = [stringValue floatValue];
    
    stringValue = dict[@"03"] ? dict[@"03"] : dict[@"3"];
    matrix.columns[0][3] = [stringValue floatValue];
    
    matrix.columns[1][0] = [dict[@"10"] floatValue];
    matrix.columns[1][1] = [dict[@"11"] floatValue];
    matrix.columns[1][2] = [dict[@"12"] floatValue];
    matrix.columns[1][3] = [dict[@"13"] floatValue];
    matrix.columns[2][0] = [dict[@"20"] floatValue];
    matrix.columns[2][1] = [dict[@"21"] floatValue];
    matrix.columns[2][2] = [dict[@"22"] floatValue];
    matrix.columns[2][3] = [dict[@"23"] floatValue];
    matrix.columns[3][0] = [dict[@"30"] floatValue];
    matrix.columns[3][1] = [dict[@"31"] floatValue];
    matrix.columns[3][2] = [dict[@"32"] floatValue];
    matrix.columns[3][3] = [dict[@"33"] floatValue];
    
    return matrix;
}

static inline matrix_float4x4 matrixFromKeyedArrayDictionary(NSDictionary *dict)
{
    matrix_float4x4 matrix;
    
    matrix.columns[0][0] = [dict[@"0"] floatValue];
    matrix.columns[0][1] = [dict[@"1"] floatValue];
    matrix.columns[0][2] = [dict[@"2"] floatValue];
    matrix.columns[0][3] = [dict[@"3"] floatValue];
    matrix.columns[1][0] = [dict[@"4"] floatValue];
    matrix.columns[1][1] = [dict[@"5"] floatValue];
    matrix.columns[1][2] = [dict[@"6"] floatValue];
    matrix.columns[1][3] = [dict[@"7"] floatValue];
    matrix.columns[2][0] = [dict[@"8"] floatValue];
    matrix.columns[2][1] = [dict[@"9"] floatValue];
    matrix.columns[2][2] = [dict[@"10"] floatValue];
    matrix.columns[2][3] = [dict[@"11"] floatValue];
    matrix.columns[3][0] = [dict[@"12"] floatValue];
    matrix.columns[3][1] = [dict[@"13"] floatValue];
    matrix.columns[3][2] = [dict[@"14"] floatValue];
    matrix.columns[3][3] = [dict[@"15"] floatValue];
    
    return matrix;
}

#endif /* ARKHelper_h */
