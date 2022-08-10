//
//  InfoViewController.swift
//  MikoGB-iOS
//
//  Created by Michael Brandt on 8/9/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

import UIKit

class InfoViewController : UIViewController {
    
    private var titleLabel: UILabel! = nil
    private var versionLabel: UILabel! = nil
    private var updateButton: UIButton! = nil
    
    private static let CheckForUpdateString = "Check For Update"
    private static let CheckingForUpdateString = "Checking For Update…"
    private static let UpdateAvailableString = "Update Available"
    private static let UpToDateString = "Up To Date"
    
    override func viewDidLoad() {
        super.viewDidLoad()
        self.view.backgroundColor = .secondarySystemBackground
        
        titleLabel = UILabel(frame: .zero)
        titleLabel.text = "MikoGB"
        titleLabel.font = UIFont.systemFont(ofSize: 20.0, weight: .bold)
        self.view.addSubview(titleLabel)
        
        versionLabel = UILabel(frame: .zero)
        let (version, build) = UpdateManager.getCurrentVersionAndBuild()
        versionLabel.text = "\(version) (Build \(build))"
        self.view.addSubview(versionLabel)
        
        var updateButtonConfig = UIButton.Configuration.filled()
        updateButtonConfig.title = InfoViewController.CheckForUpdateString
        updateButtonConfig.buttonSize = .large
        updateButtonConfig.imagePadding = 5.0
        let updateButtonAction = UIAction { [weak self] _ in
            self?._checkForUpdate()
        }
        updateButton = UIButton(configuration: updateButtonConfig, primaryAction: updateButtonAction)
        self.view.addSubview(updateButton)
        
        let xButtonImage = UIImage(systemName: "xmark.circle.fill")?.withConfiguration(UIImage.SymbolConfiguration.init(hierarchicalColor: .lightGray))
        let xButtonAction = UIAction { [weak self] _ in
            self?.dismiss(animated: true)
        }
        let xButton = UIBarButtonItem(title: nil, image: xButtonImage, primaryAction: xButtonAction, menu: nil)
        self.navigationItem.rightBarButtonItem = xButton
    }
    
    override func viewWillLayoutSubviews() {
        super.viewWillLayoutSubviews()
        
        var availableBounds = self.view.bounds.inset(by: self.view.layoutMargins)
        let titleSize = titleLabel.sizeThatFits(availableBounds.size)
        let titleX = availableBounds.minX + ((availableBounds.size.width - titleSize.width) / 2.0)
        let titleY = (availableBounds.size.height * 0.10) // 10% down
        let titleFrame = CGRect(x: titleX, y: titleY, width: titleSize.width, height: titleSize.height)
        titleLabel.frame = titleFrame
        
        availableBounds.origin.y = titleFrame.maxY
        availableBounds.size.height -= titleFrame.maxY
        let versionSize = versionLabel.sizeThatFits(availableBounds.size)
        let versionX = availableBounds.minX + ((availableBounds.size.width - versionSize.width) / 2.0)
        let versionY = availableBounds.minY + 10.0
        let versionFrame = CGRect(x: versionX, y: versionY, width: versionSize.width, height: versionSize.height)
        versionLabel.frame = versionFrame
        
        availableBounds.size.height -= (versionFrame.maxY - (availableBounds.minY))
        availableBounds.origin.y = versionFrame.maxY
        let minButtonSize = updateButton.sizeThatFits(availableBounds.size)
        let buttonSize = CGSize(width: max(minButtonSize.width, availableBounds.width), height: minButtonSize.height)
        let buttonX = availableBounds.minX + ((availableBounds.size.width - buttonSize.width) / 2.0)
        let buttonY = availableBounds.maxY - buttonSize.height
        let buttonFrame = CGRect(x: buttonX, y: buttonY, width: buttonSize.width, height: buttonSize.height)
        updateButton.frame = buttonFrame
    }
    
    private func _checkForUpdate() {
        if var config = self.updateButton.configuration {
            config.showsActivityIndicator = true
            config.title = InfoViewController.CheckingForUpdateString
            self.updateButton.configuration = config
        }
        self.updateButton.isEnabled = false
        
        let updateType: UpdateManager.UpdateType = SettingsManager.sharedInstance.checkForStagingUpdates ? .staging : .release
        let checkStartTime = CFAbsoluteTimeGetCurrent()
        UpdateManager.checkForUpdate(type: updateType, force: true) { [weak self] result in
            let diff: Double = CFAbsoluteTimeGetCurrent() - checkStartTime
            if diff >= 1.0 {
                DispatchQueue.main.async { [weak self] in
                    self?._updateCheckDone(result)
                }
            } else {
                let diffMilli = Int((1.0 - diff) * 1000.0)
                DispatchQueue.main.asyncAfter(deadline: .now() + .milliseconds(diffMilli)) { [weak self] in
                    self?._updateCheckDone(result)
                }
            }
        }
    }
    
    private func _updateCheckDone(_ result: Result<(Bool, String), Error>) {
        switch result {
        case .success(let (hasUpdate, versionName)):
            _handleUpdateCheckSuccess(hasUpdate: hasUpdate, versionName: versionName)
        case .failure(let error):
            _handleUpdateCheckFailure(error)
        }
        
        
    }
    
    private func _handleUpdateCheckSuccess(hasUpdate: Bool, versionName: String) {
        if var config = self.updateButton.configuration {
            config.showsActivityIndicator = false
            let image = UIImage(systemName: "checkmark.circle")?.withConfiguration(UIImage.SymbolConfiguration(hierarchicalColor: .green))
            config.image = image
            config.title = hasUpdate ? InfoViewController.UpdateAvailableString : InfoViewController.UpToDateString
            self.updateButton.configuration = config
        }
        self.updateButton.isEnabled = false
        
        if !hasUpdate {
            // successfully found that there's nothing to do
            return
        }
        
        let alert = UIAlertController(title: "Update Available", message: "An update is available to \(versionName)", preferredStyle: .alert)
        alert.addAction(UIAlertAction(title: "Not Now", style: .cancel))
        alert.addAction(UIAlertAction(title: "Install", style: .default, handler: { _ in
            if let url = UpdateManager.updateURL() {
                self.view.window?.windowScene?.open(url, options: nil)
            }
        }))
        self.present(alert, animated: true)
    }
    
    private func _handleUpdateCheckFailure(_ error: Error) {
        if var config = self.updateButton.configuration {
            config.showsActivityIndicator = false
            let image = UIImage(systemName: "xmark.octagon")?.withConfiguration(UIImage.SymbolConfiguration(hierarchicalColor: .red))
            config.image = image
            self.updateButton.configuration = config
        }
        self.updateButton.isEnabled = true
                
        let message = "\(error.localizedDescription)"
        let alert = UIAlertController(title: "Error Checking for Update", message: message, preferredStyle: .alert)
        alert.addAction(UIAlertAction(title: "OK", style: .default))
        self.present(alert, animated: true)
    }
}
