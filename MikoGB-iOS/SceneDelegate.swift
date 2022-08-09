//
//  SceneDelegate.swift
//  MikoGB-iOS
//
//  Created by Michael Brandt on 3/16/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

import UIKit

class SceneDelegate: UIResponder, UIWindowSceneDelegate {

    var window: UIWindow?


    func scene(_ scene: UIScene, willConnectTo session: UISceneSession, options connectionOptions: UIScene.ConnectionOptions) {
        // Use this method to optionally configure and attach the UIWindow `window` to the provided UIWindowScene `scene`.
        // If using a storyboard, the `window` property will automatically be initialized and attached to the scene.
        // This delegate does not imply the connecting scene or session are new (see `application:configurationForConnectingSceneSession` instead).
        guard let _ = (scene as? UIWindowScene) else { return }
    }

    func sceneDidDisconnect(_ scene: UIScene) {
        // Called as the scene is being released by the system.
        // This occurs shortly after the scene enters the background, or when its session is discarded.
        // Release any resources associated with this scene that can be re-created the next time the scene connects.
        // The scene may re-connect later, as its session was not necessarily discarded (see `application:didDiscardSceneSessions` instead).
    }

    func sceneDidBecomeActive(_ scene: UIScene) {
        // Called when the scene has moved from an inactive state to an active state.
        // Use this method to restart any tasks that were paused (or not yet started) when the scene was inactive.
        UpdateManager.checkForUpdate { [weak self] result in
            switch result {
            case .success(let (hasUpdate, targetVersion)):
                if hasUpdate {
                    self?._presentUpdateAlert(onScene: scene, title: "Update Available", message: "An update is available to \(targetVersion)")
                }
            case .failure(_):
                break
            }
        }
        
        UserIdentityController.sharedIdentityController.ensureRegistration()
        UserIdentityController.sharedIdentityController.checkIn()
    }

    func sceneWillResignActive(_ scene: UIScene) {
        // Called when the scene will move from an active state to an inactive state.
        // This may occur due to temporary interruptions (ex. an incoming phone call).
    }

    func sceneWillEnterForeground(_ scene: UIScene) {
        // Called as the scene transitions from the background to the foreground.
        // Use this method to undo the changes made on entering the background.
    }

    func sceneDidEnterBackground(_ scene: UIScene) {
        // Called as the scene transitions from the foreground to the background.
        // Use this method to save data, release shared resources, and store enough scene-specific state information
        // to restore the scene back to its current state.
    }
    
    private func _presentUpdateAlert(onScene scene: UIScene, title: String, message: String) {
        if let windowScene = scene as? UIWindowScene, let vc = windowScene.keyWindow?.rootViewController {
            let alert = UIAlertController(title: title, message: message, preferredStyle: .alert)
            alert.addAction(UIAlertAction(title: "Not Now", style: .cancel))
            alert.addAction(UIAlertAction(title: "Install", style: .default, handler: { _ in
                if let url = UpdateManager.updateURL() {
                    windowScene.open(url, options: nil)
                }
            }))
            vc.present(alert, animated: true)
        } else {
            preconditionFailure("We expect to always be using window scenes")
        }
    }
    
    private func _presentErrorAlert(onScene scene: UIScene, title: String, message: String) {
        if let windowScene = scene as? UIWindowScene, let vc = windowScene.keyWindow?.rootViewController {
            let alert = UIAlertController(title: title, message: message, preferredStyle: .alert)
            alert.addAction(UIAlertAction(title: "OK", style: .default))
            vc.present(alert, animated: true)
        } else {
            preconditionFailure("We expect to always be using window scenes")
        }
    }
    
    func scene(_ scene: UIScene, openURLContexts URLContexts: Set<UIOpenURLContext>) {
        print("Opening URL contexts \(URLContexts)")
        for context in URLContexts {
            let url = context.url
            let pathExtension = url.pathExtension
            if pathExtension == "gb" || pathExtension == "gbc" {
                do {
                    try PersistenceManager.installROM(url)
                } catch {
                    _presentErrorAlert(onScene: scene, title: "Unable to install", message: error.localizedDescription)
                }
            } else {
                _presentErrorAlert(onScene: scene, title: "Unrecognized file type", message: "How did you get here??")
            }
        }
    }
}
