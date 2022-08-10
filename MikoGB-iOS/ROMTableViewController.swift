//
//  ROMTableViewController.swift
//  MikoGB-iOS
//
//  Created by Michael Brandt on 3/19/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

import UIKit

class ROMTableViewController: UITableViewController {
    
    private let persistenceManager = PersistenceManager()
    private var romsList = [(String, String)]()
    override func viewDidLoad() {
        super.viewDidLoad()
        
        self.title = "ROMs"
        if let roms = try? persistenceManager.getROMs() {
            romsList = roms
        } else {
            romsList = [(String, String)]()
        }
        
        tableView.register(UITableViewCell.self, forCellReuseIdentifier: "Cell")
        
        let infoImage = UIImage(systemName: "info.circle")
        let infoAction = UIAction { [weak self] _ in
            self?._handleInfoAction()
        }
        let infoButton = UIBarButtonItem(title: nil, image: infoImage, primaryAction: infoAction, menu: nil)
        self.navigationItem.leftBarButtonItem = infoButton
        
        let settingsImage = UIImage(systemName: "gearshape")
        let settingsAction = UIAction { [weak self] _ in
            self?._handleSettingsAction()
        }
        let settingsButton = UIBarButtonItem(title: nil, image: settingsImage, primaryAction: settingsAction, menu: nil)
        self.navigationItem.rightBarButtonItem = settingsButton
    }
    
    // MARK: - Actions
    
    private func _handleInfoAction() {
        let infoVC = InfoViewController()
        let navigationController = UINavigationController(rootViewController: infoVC)
        navigationController.modalPresentationStyle = .formSheet
        if let sheet = navigationController.sheetPresentationController {
            sheet.detents = [.medium()]
            //TODO: This doesn't seem to work with only medium detent. See if it does in iOS 16
//            sheet.prefersEdgeAttachedInCompactHeight = true
//            sheet.widthFollowsPreferredContentSizeWhenEdgeAttached = true
        }
        self.present(navigationController, animated: true)
    }
    
    private func _handleSettingsAction() {
        let settingsVC = SettingsTableViewController()
        let navigationController = UINavigationController(rootViewController: settingsVC)
        if let sheet = navigationController.sheetPresentationController {
            sheet.detents = [.medium(), .large()]
            sheet.prefersScrollingExpandsWhenScrolledToEdge = false
            sheet.prefersEdgeAttachedInCompactHeight = true
            sheet.widthFollowsPreferredContentSizeWhenEdgeAttached = true
        }
        self.present(navigationController, animated: true)
    }

    // MARK: - Table view data source

    override func numberOfSections(in tableView: UITableView) -> Int {
        return 1
    }

    override func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return romsList.count
    }

    override func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let cell = tableView.dequeueReusableCell(withIdentifier: "Cell", for: indexPath)
        let (name, _) = romsList[indexPath.row]
        var configuration = cell.defaultContentConfiguration()
        configuration.text = name
        cell.contentConfiguration = configuration

        return cell
    }
    
    // MARK: - Table view delegate

    override func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
        let (_, romPath) = romsList[indexPath.row]
        let url = URL(fileURLWithPath: romPath)
        let gameVC = GameViewController(url, persistenceManager: persistenceManager)
        self.navigationController?.pushViewController(gameVC, animated: true)
    }
    
    override func tableView(_ tableView: UITableView, trailingSwipeActionsConfigurationForRowAt indexPath: IndexPath) -> UISwipeActionsConfiguration? {
        
        let deleteAction = UIContextualAction(style: .destructive, title: "Delete") { _, _, completionHandler in
            print("Delete running!")
            completionHandler(true)
        }
        
        let deleteConfig = UISwipeActionsConfiguration(actions: [deleteAction])
        return deleteConfig
    }

}
