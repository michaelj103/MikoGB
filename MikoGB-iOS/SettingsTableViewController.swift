//
//  SettingsTableViewController.swift
//  MikoGB-iOS
//
//  Created by Michael Brandt on 8/9/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

import UIKit

class SettingsTableViewController : UITableViewController {
    
    private var dataSource: SettingsDiffableDataSource! = nil
    private var showDebugSection = false {
        didSet {
            _updateDataSource()
        }
    }
    
    static func presentModal(on viewController: UIViewController) {
        let settingsVC = SettingsTableViewController()
        let navigationController = UINavigationController(rootViewController: settingsVC)
        if let sheet = navigationController.sheetPresentationController {
            sheet.detents = [.medium(), .large()]
            sheet.prefersScrollingExpandsWhenScrolledToEdge = false
            sheet.prefersEdgeAttachedInCompactHeight = true
            sheet.widthFollowsPreferredContentSizeWhenEdgeAttached = true
        }
        viewController.present(navigationController, animated: true)
    }
    
    required init() {
        super.init(style: .insetGrouped)
    }
    
    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        self.title = "Settings"
        
        for rowType in SettingsRow.RowType.allCases {
            let cellClass = rowType.cellClass()
            let identifier = rowType.cellIdentifier()
            tableView.register(cellClass, forCellReuseIdentifier: identifier)
        }
        
        let doneAction = UIAction { [weak self] _ in
            self?.dismiss(animated: true)
        }
        let doneButton = UIBarButtonItem(title: "Done", image: nil, primaryAction: doneAction, menu: nil)
        doneButton.style = .done
        self.navigationItem.rightBarButtonItem = doneButton
        
        dataSource = SettingsDiffableDataSource(tableView: self.tableView, cellProvider: { tableView, indexPath, itemIdentifier in
            itemIdentifier.provideRow(tableView, indexPath: indexPath) { [weak self] taggable in
                self?._commitValueChange(taggable)
            }
        })
        
        let snapshot = _generateDataSourceSnapshot()
        dataSource.apply(snapshot)
        
        UserIdentityController.sharedIdentityController.getDebugAuthorization { [weak self] authorized in
            self?.showDebugSection = authorized
        }
    }
        
    private func _generateDataSourceSnapshot() -> NSDiffableDataSourceSnapshot<SettingsSection, SettingsRow> {
        var snapshot = NSDiffableDataSourceSnapshot<SettingsSection,SettingsRow>()
        
        // Audio
        var audioSection = SettingsSection("AudioSection")
        audioSection.title = "Audio"
        snapshot.appendSections([audioSection])
        var generateAudio = SettingsRow(.generateAudioSwitch, type: .switchRow)
        generateAudio.title = "Generate Audio"
        var respectMuteSwitch = SettingsRow(.respectsMuteSwitch, type: .switchRow)
        respectMuteSwitch.title = "Respect Mute Switch"
        snapshot.appendItems([generateAudio, respectMuteSwitch], toSection: audioSection)
        
        if showDebugSection {
            var debugSection = SettingsSection("DebugSection")
            debugSection.title = "Debug"
            snapshot.appendSections([debugSection])
            var stagingSwitch = SettingsRow(.debugCheckForStagingUpdates, type: .switchRow)
            stagingSwitch.title = "Check For Staging Updates"
            var serverSwitch = SettingsRow(.debugUseCustomServer, type: .switchRow)
            serverSwitch.title = "Use Custom Server"
            snapshot.appendItems([stagingSwitch, serverSwitch], toSection: debugSection)
            
            var customServerRow = SettingsRow(.debugCustomServerString, type: .valueRow)
            customServerRow.title = "Custom Server"
            customServerRow.version = serverVersion
            snapshot.appendItems([customServerRow], toSection: debugSection)
        }
        
        return snapshot
    }
    
    private func _updateDataSource(animated: Bool = true) {
        let snapshot = _generateDataSourceSnapshot()
        dataSource.apply(snapshot, animatingDifferences: animated)
    }
    
    private func _commitValueChange(_ taggable: RowTaggable) {
        let snapshot = dataSource.snapshot()
        let indexPath = taggable.rowTag
        let sectionIdentifier = snapshot.sectionIdentifiers[indexPath.section]
        let rowIdentifier = snapshot.itemIdentifiers(inSection: sectionIdentifier)[indexPath.row]
        let requiresReload: Bool
        switch rowIdentifier.type {
        case .switchRow:
            requiresReload = _commitValueForSwitchRow(rowIdentifier, sender: taggable as! UISwitch)
        case .valueRow:
            requiresReload = false
            break
        }
        
        if requiresReload {
            DispatchQueue.main.async {
                self._updateDataSource()
            }
        }
    }
    
    private func _commitValueForSwitchRow(_ row: SettingsRow, sender: UISwitch) -> Bool {
        var requiresReload = false
        switch row.identifier {
        case .generateAudioSwitch:
            SettingsManager.sharedInstance.shouldGenerateAudio = sender.isOn
        case .respectsMuteSwitch:
            SettingsManager.sharedInstance.shouldRespectMuteSwitch = sender.isOn
        case .debugCheckForStagingUpdates:
            SettingsManager.sharedInstance.checkForStagingUpdates = sender.isOn
        case .debugUseCustomServer:
            SettingsManager.sharedInstance.useServerOverride = sender.isOn
            _ensureRegistrationIfNecessary()
            requiresReload = true
        case .debugCustomServerString:
            preconditionFailure()
        }
        return requiresReload
    }
    
    override func tableView(_ tableView: UITableView, willSelectRowAt indexPath: IndexPath) -> IndexPath? {
        if let identifier = dataSource.itemIdentifier(for: indexPath) {
            switch identifier.type {
            case .switchRow:
                return nil
            case .valueRow:
                return indexPath
            }
        } else {
            return nil
        }
    }
    
    override func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
        if let identifier = dataSource.itemIdentifier(for: indexPath) {
            switch identifier.type {
            case .switchRow:
                break
            case .valueRow:
                _performValueCellAction(identifier)
            }
        }
        tableView.deselectRow(at: indexPath, animated: true)
    }
    
    private func _performValueCellAction(_ identifier: SettingsRow) {
        switch identifier.identifier {
        case .generateAudioSwitch, .respectsMuteSwitch, .debugCheckForStagingUpdates, .debugUseCustomServer:
            break
        case .debugCustomServerString:
            _getCustomServerValue()
        }
    }
    
    private func _getCustomServerValue() {
        let alert = UIAlertController(title: "Enter Server", message: "Include port, if necessary", preferredStyle: .alert)
        alert.addTextField { textField in
            textField.placeholder = "hostname:port"
        }
        
        let okAction = UIAlertAction(title: "OK", style: .default) { [weak self] _ in
            let server = alert.textFields?.first?.text
            let resolvedServer = (server?.count ?? 0) > 0 ? server : nil
            self?._setCustomServer(resolvedServer)
        }
        alert.addAction(okAction)
        
        let cancelAction = UIAlertAction(title: "Cancel", style: .cancel)
        alert.addAction(cancelAction)
        self.present(alert, animated: true)
    }
    
    private var serverVersion = 1
    private func _setCustomServer(_ value: String?) {
        SettingsManager.sharedInstance.customServer = value
        serverVersion += 1
        _updateDataSource(animated: false)
        _ensureRegistrationIfNecessary()
    }
    
    private func _ensureRegistrationIfNecessary() {
        let customServer = SettingsManager.sharedInstance.customServer
        let wantsOverride = SettingsManager.sharedInstance.useServerOverride && customServer != nil
        let hasOverride = ServerConfiguration.hasHostOverride()
        
        var ensureRegistration = false
        if wantsOverride && !hasOverride {
            let success = ServerConfiguration.setTemporaryHost(customServer!)
            ensureRegistration = success
            if !success {
                // TODO: Present failure alert
            }
        } else if !wantsOverride && hasOverride {
            ServerConfiguration.stopTemporaryOverride()
            ensureRegistration = true
        }
        
        if ensureRegistration {
            UserIdentityController.sharedIdentityController.ensureRegistration(force: true, completion: nil)
        }
    }
}

fileprivate class SettingsDiffableDataSource : UITableViewDiffableDataSource<SettingsSection, SettingsRow> {
    override func tableView(_ tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
        let sectionIdentifier = self.sectionIdentifier(for: section)
        return sectionIdentifier?.title
    }
    
    override func tableView(_ tableView: UITableView, titleForFooterInSection section: Int) -> String? {
        let sectionIdentifier = self.sectionIdentifier(for: section)
        return sectionIdentifier?.footer
    }
    
    
}

fileprivate struct SettingsSection: Hashable {
    let identifier: String
    var title: String? = nil
    var footer: String? = nil
    
    init(_ identifier: String) {
        self.identifier = identifier
    }
    
    func hash(into hasher: inout Hasher) {
        hasher.combine(identifier)
    }
}

fileprivate enum RowIdentifier: String {
    case generateAudioSwitch
    case respectsMuteSwitch
    case debugCheckForStagingUpdates
    case debugUseCustomServer
    case debugCustomServerString
}

fileprivate struct SettingsRow: Hashable {
    let identifier: RowIdentifier
    let type: RowType
    var title: String? = nil
    var version: Int = 1
    
    init(_ identifier: RowIdentifier, type: RowType) {
        self.identifier = identifier
        self.type = type
    }
    
    enum RowType: CaseIterable {
        case switchRow
        case valueRow
        
        func cellIdentifier() -> String {
            switch self {
            case .switchRow:
                return "SwitchCell"
            case .valueRow:
                return "ValueCell"
            }
        }
        
        func cellClass() -> UITableViewCell.Type {
            switch self {
            case .switchRow:
                return UITableViewCell.self
            case .valueRow:
                return UITableViewCell.self
            }
        }
    }
    
    func provideRow(_ tableView: UITableView, indexPath: IndexPath, commitBlock: @escaping (RowTaggable) -> Void) -> UITableViewCell? {
        let cell = tableView.dequeueReusableCell(withIdentifier: type.cellIdentifier(), for: indexPath)
        switch type {
        case .switchRow:
            _configureSwitchCell(cell, indexPath: indexPath, commitBlock: commitBlock)
        case .valueRow:
            _configureValueCell(cell)
        }
        
        return cell
    }
    
    private func _configureSwitchCell(_ cell: UITableViewCell, indexPath: IndexPath, commitBlock: @escaping (RowTaggable) -> Void) {
        var configuration = cell.defaultContentConfiguration()
        configuration.text = title
        cell.contentConfiguration = configuration
        let switchView: UISwitch
        if let accessoryView = cell.accessoryView as? UISwitch {
            switchView = accessoryView
        } else {
            let switchAction = UIAction { action in
                if let taggableSender = action.sender as? RowTaggable {
                    commitBlock(taggableSender)
                }
            }
            switchView = UISwitch(frame: .zero, primaryAction: switchAction)
            cell.accessoryView = switchView
        }
        
        switchView.tag = UISwitch.EncodeRowTag(indexPath)
        let value: Bool
        switch identifier {
        case .generateAudioSwitch:
            value = SettingsManager.sharedInstance.shouldGenerateAudio
        case .respectsMuteSwitch:
            value = SettingsManager.sharedInstance.shouldRespectMuteSwitch
        case .debugCheckForStagingUpdates:
            value = SettingsManager.sharedInstance.checkForStagingUpdates
        case .debugUseCustomServer:
            value = SettingsManager.sharedInstance.useServerOverride
        case .debugCustomServerString:
            // not a switch cell
            preconditionFailure()
        }
        switchView.isOn = value
    }
    
    private func _configureValueCell(_ cell: UITableViewCell) {
        var configuration = UIListContentConfiguration.valueCell()
        configuration.text = title
        let secondaryText: String
        switch identifier {
        case .generateAudioSwitch, .respectsMuteSwitch, .debugCheckForStagingUpdates, .debugUseCustomServer:
            preconditionFailure()
        case .debugCustomServerString:
            secondaryText = SettingsManager.sharedInstance.customServer ?? "None"
        }
        configuration.secondaryText = secondaryText
        cell.contentConfiguration = configuration
    }
}

protocol RowTaggable {
    static func EncodeRowTag(_ indexPath: IndexPath) -> Int
    static func DecodeRowTag(_ tag: Int) -> IndexPath
    var rowTag: IndexPath { get set }
}

extension RowTaggable {
    static func EncodeRowTag(_ indexPath: IndexPath) -> Int {
        precondition(indexPath.row < 10000)
        let val = (indexPath.section * 10000) + indexPath.row
        return val
    }
    
    static func DecodeRowTag(_ tag: Int) -> IndexPath {
        let row = tag % 10000
        let section = tag / 10000
        return IndexPath(row: row, section: section)
    }
}

extension UISwitch: RowTaggable {
    var rowTag: IndexPath {
        get { UISwitch.DecodeRowTag(self.tag) }
        set { self.tag = UISwitch.EncodeRowTag(newValue) }
    }
}
