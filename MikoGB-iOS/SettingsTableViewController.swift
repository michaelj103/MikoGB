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
            itemIdentifier.provideRow(tableView, indexPath: indexPath)
        })
        
        let snapshot = _currentDataSourceSnapshot()
        dataSource.apply(snapshot)
    }
    
    private func _currentDataSourceSnapshot() -> NSDiffableDataSourceSnapshot<SettingsSection, SettingsRow> {
        var snapshot = NSDiffableDataSourceSnapshot<SettingsSection,SettingsRow>()
        var audioSection = SettingsSection("AudioSection")
        audioSection.title = "Audio"
        snapshot.appendSections([audioSection])
        
        var generateAudio = SettingsRow("GenerateAudioSwitchRow", type: .switchRow)
        generateAudio.title = "Generate Audio"
        var respectMuteSwitch = SettingsRow("RespectsMuteSwitchRow", type: .switchRow)
        respectMuteSwitch.title = "Respect Mute Switch"
        snapshot.appendItems([generateAudio, respectMuteSwitch], toSection: audioSection)
        
        return snapshot
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

fileprivate struct SettingsRow: Hashable {
    let identifier: String
    let type: RowType
    var title: String? = nil
    
    init(_ identifier: String, type: RowType) {
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
    
    func provideRow(_ tableView: UITableView, indexPath: IndexPath) -> UITableViewCell? {
        let cell = tableView.dequeueReusableCell(withIdentifier: type.cellIdentifier(), for: indexPath)
        var configuration = cell.defaultContentConfiguration()
        configuration.text = title
        cell.contentConfiguration = configuration
        let switchView: UISwitch
        if let accessoryView = cell.accessoryView as? UISwitch {
            switchView = accessoryView
        } else {
            switchView = UISwitch()
            cell.accessoryView = switchView
        }
        return cell
    }
}
