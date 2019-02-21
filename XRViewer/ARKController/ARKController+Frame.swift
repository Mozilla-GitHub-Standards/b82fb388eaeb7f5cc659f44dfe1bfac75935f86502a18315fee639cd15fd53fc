extension ARKController {
    
    func updateARKData(with frame: ARFrame) {
        
        synced(self) {
            if request == nil {
                return
            }
            guard let controller = controller, let configuration = configuration else { return }
            var newData = [AnyHashable: Any].init(minimumCapacity: 3) // max request object
            let ts = Int(frame.timestamp * 1000.0)
            newData["timestamp"] = ts
            
            // status of ARKit World Mapping
            newData[WEB_AR_WORLDMAPPING_STATUS_MESSAGE] = worldMappingState(frame)
            
            if (request[WEB_AR_LIGHT_INTENSITY_OPTION] as? NSNumber)?.boolValue ?? false {
                newData[WEB_AR_LIGHT_INTENSITY_OPTION] = frame.lightEstimate?.ambientIntensity ?? 0.0
                
                let lightDictionary = NSMutableDictionary.init()
                lightDictionary[WEB_AR_LIGHT_INTENSITY_OPTION] = frame.lightEstimate?.ambientIntensity ?? 0.0
                lightDictionary[WEB_AR_LIGHT_AMBIENT_COLOR_TEMPERATURE_OPTION] = frame.lightEstimate?.ambientColorTemperature ?? 0.0
                
                if frame.lightEstimate is ARDirectionalLightEstimate, let directionalLightEstimate = frame.lightEstimate as? ARDirectionalLightEstimate {
                    lightDictionary[WEB_AR_PRIMARY_LIGHT_DIRECTION_OPTION] = [
                        "x": directionalLightEstimate.primaryLightDirection.x,
                        "y": directionalLightEstimate.primaryLightDirection.y,
                        "z": directionalLightEstimate.primaryLightDirection.z
                    ]
                    lightDictionary[WEB_AR_PRIMARY_LIGHT_INTENSITY_OPTION] = directionalLightEstimate.primaryLightIntensity
                }
                newData[WEB_AR_LIGHT_OBJECT_OPTION] = lightDictionary
            }
            if (request[WEB_AR_CAMERA_OPTION] as? NSNumber)?.boolValue ?? false {
                let size: CGSize = controller.getRenderView().frame.size
                var projectionMatrix = matrix_float4x4()
                projectionMatrix = frame.camera.projectionMatrix(for: interfaceOrientation,
                                                                 viewportSize: size,
                                                                 zNear: CGFloat(AR_CAMERA_PROJECTION_MATRIX_Z_NEAR),
                                                                 zFar: CGFloat(AR_CAMERA_PROJECTION_MATRIX_Z_FAR))
                if let projMatrix = arrayFromMatrix4x4(projectionMatrix) {
                    newData[WEB_AR_PROJ_CAMERA_OPTION] = projMatrix
                }
                
                var viewMatrix = matrix_float4x4()
                viewMatrix = frame.camera.viewMatrix(for: interfaceOrientation)
                let modelMatrix: matrix_float4x4 = viewMatrix.inverse
                
                if let modelArray = arrayFromMatrix4x4(modelMatrix) {
                    newData[WEB_AR_CAMERA_TRANSFORM_OPTION] = modelArray
                }
                if let viewArray = arrayFromMatrix4x4(viewMatrix) {
                    newData[WEB_AR_CAMERA_VIEW_OPTION] = viewArray
                }
            }
            if (request[WEB_AR_3D_OBJECTS_OPTION] as? NSNumber)?.boolValue ?? false {
                let anchorsArray = currentAnchorsArray()
                newData[WEB_AR_3D_OBJECTS_OPTION] = anchorsArray
                
                // Prepare the objectsRemoved array
                if let removedObjects = removedAnchorsSinceLastFrame.copy() as? NSArray {
                    removedAnchorsSinceLastFrame.removeAllObjects()
                    newData[WEB_AR_3D_REMOVED_OBJECTS_OPTION] = removedObjects
                }
                
                // Prepare the newObjects array
                if let newObjects = addedAnchorsSinceLastFrame.copy() as? NSArray {
                    addedAnchorsSinceLastFrame.removeAllObjects()
                    newData[WEB_AR_3D_NEW_OBJECTS_OPTION] = newObjects
                }
            }
            if computerVisionDataEnabled && computerVisionFrameRequested {
                let cameraInformation = NSMutableDictionary.init()
                let cameraImageResolution: CGSize = frame.camera.imageResolution
                cameraInformation["cameraImageResolution"] = [
                    "width": cameraImageResolution.width,
                    "height": cameraImageResolution.height
                ]
                
                let cameraIntrinsics = frame.camera.intrinsics
                var resizedCameraIntrinsics = frame.camera.intrinsics
                resizedCameraIntrinsics.columns.0.x = cameraIntrinsics.columns.0.x / computerVisionImageScaleFactor
                resizedCameraIntrinsics.columns.0.y = cameraIntrinsics.columns.0.y / computerVisionImageScaleFactor
                resizedCameraIntrinsics.columns.0.z = cameraIntrinsics.columns.0.z / computerVisionImageScaleFactor
                resizedCameraIntrinsics.columns.1.x = cameraIntrinsics.columns.1.x / computerVisionImageScaleFactor
                resizedCameraIntrinsics.columns.1.y = cameraIntrinsics.columns.1.y / computerVisionImageScaleFactor
                resizedCameraIntrinsics.columns.1.z = cameraIntrinsics.columns.1.z / computerVisionImageScaleFactor
                resizedCameraIntrinsics.columns.2.x = cameraIntrinsics.columns.2.x / computerVisionImageScaleFactor
                resizedCameraIntrinsics.columns.2.y = cameraIntrinsics.columns.2.y / computerVisionImageScaleFactor
                resizedCameraIntrinsics.columns.2.z = cameraIntrinsics.columns.2.z / computerVisionImageScaleFactor
                resizedCameraIntrinsics.columns.2.z = 1.0
                cameraInformation["cameraIntrinsics"] = arrayFromMatrix3x3(resizedCameraIntrinsics)
                
                // Get the projection matrix
                let viewportSize: CGSize = controller.getRenderView().frame.size
                var projectionMatrix = matrix_float4x4()
                projectionMatrix = frame.camera.projectionMatrix(for: interfaceOrientation,
                                                                 viewportSize: viewportSize,
                                                                 zNear: CGFloat(AR_CAMERA_PROJECTION_MATRIX_Z_NEAR),
                                                                 zFar: CGFloat(AR_CAMERA_PROJECTION_MATRIX_Z_FAR))
                cameraInformation["projectionMatrix"] = arrayFromMatrix4x4(projectionMatrix)
                
                // Get the view matrix
                var viewMatrix = matrix_float4x4()
                viewMatrix = frame.camera.viewMatrix(for: interfaceOrientation)
                cameraInformation["viewMatrix"] = arrayFromMatrix4x4(viewMatrix)
                cameraInformation["inverse_viewMatrix"] = arrayFromMatrix4x4(viewMatrix.inverse)
                
                // Send also the interface orientation
                cameraInformation["interfaceOrientation"] = interfaceOrientation.rawValue
                
                var cvInformation = [AnyHashable: Any]()
                let frameInformation = NSMutableDictionary.init()
                let timestamp = Int(frame.timestamp * 1000.0)
                frameInformation["timestamp"] = timestamp
                
                // TODO: prepare depth data
                frameInformation["capturedDepthData"] = nil
                frameInformation["capturedDepthDataTimestamp"] = nil
                
                // Computer vision data
                updateBase64Buffers(from: frame.capturedImage)
                
                let lumaBufferDictionary = NSMutableDictionary.init()
                lumaBufferDictionary["size"] = [
                    "width": lumaBufferSize.width,
                    "height": lumaBufferSize.height,
                    "bytesPerRow": lumaBufferSize.width * CGFloat(MemoryLayout<Pixel_8>.size),
                    "bytesPerPixel": MemoryLayout<Pixel_8>.size
                ]
                lumaBufferDictionary["buffer"] = lumaBase64StringBuffer
                
                
                let chromaBufferDictionary = NSMutableDictionary.init()
                chromaBufferDictionary["size"] = [
                    "width": chromaBufferSize.width,
                    "height": chromaBufferSize.height,
                    "bytesPerRow": chromaBufferSize.width * CGFloat(MemoryLayout<Pixel_16U>.size),
                    "bytesPerPixel": MemoryLayout<Pixel_16U>.size
                ]
                chromaBufferDictionary["buffer"] = chromaBase64StringBuffer
                
                frameInformation["buffers"] = [lumaBufferDictionary, chromaBufferDictionary]
                frameInformation["pixelFormatType"] = string(for: CVPixelBufferGetPixelFormatType(frame.capturedImage))
                
                cvInformation["frame"] = frameInformation
                cvInformation["camera"] = cameraInformation
                
                os_unfair_lock_lock(&(lock))
                computerVisionData = cvInformation
                os_unfair_lock_unlock(&(lock))
            }
            
            newData[WEB_AR_3D_GEOALIGNED_OPTION] = NSNumber(value: configuration.worldAlignment == .gravityAndHeading ? true : false)
            newData[WEB_AR_3D_VIDEO_ACCESS_OPTION] = NSNumber(value: computerVisionDataEnabled ? true : false)
            
            os_unfair_lock_lock(&(lock))
            arkData = newData
            os_unfair_lock_unlock(&(lock))
        }
    }
    
    func synced(_ lock: Any, closure: () -> ()) {
        objc_sync_enter(lock)
        closure()
        objc_sync_exit(lock)
    }
}