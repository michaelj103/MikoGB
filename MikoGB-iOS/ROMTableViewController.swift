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
    }
    
    // MARK: - Actions
    
    private func _handleInfoAction() {
        let infoVC = InfoViewController()
        if let sheetPresentationController = infoVC.sheetPresentationController {
            sheetPresentationController.detents = [.medium()]
        }
        self.present(infoVC, animated: true)
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
