//
//  DebuggerInstructionController.swift
//  MikoGB
//
//  Created by Michael Brandt on 2/20/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

import Cocoa

class DebuggerInstructionController : NSObject, NSTableViewDataSource, GBEngineObserver {
    var engine: GBEngine? {
        willSet {
            engine?.unregisterObserver(self)
        }
        didSet {
            engine?.register(self)
            currentRunnable = engine?.isRunnable ?? false
            _reloadData()
        }
    }
    
    var tableView: NSTableView? {
        willSet {
            tableView?.dataSource = nil
        }
        didSet {
            tableView?.dataSource = self
            _updateTable()
        }
    }
    
    private func _updateTable() {
        guard let table = tableView else {
            return
        }
        
        let columns = table.tableColumns
        columns[0].title = "Address"
        columns[1].title = "Instruction"
    }
    
    private var currentRunnable = false
    func engine(_ engine: GBEngine, runnableDidChange isRunnable: Bool) {
        currentRunnable = isRunnable
        _reloadData()
    }
    
    func didUpdateSuspendedState(for engine: GBEngine) {
        _reloadData()
    }
    
    private var addresses = [String]()
    private var instructions = [String]()
    private func _reloadData() {
        addresses = []
        instructions = []
        guard let engine = engine else {
            tableView?.reloadData()
            return
        }
        
        if currentRunnable {
            tableView?.reloadData()
            return
        }

        let disas = engine.disassembledInstructions()
        var i = 0
        while i < disas.count {
            addresses.append(disas[i])
            instructions.append(disas[i+1])
            i += 2
        }
        tableView?.reloadData()
    }
    
    func numberOfRows(in tableView: NSTableView) -> Int {
        if currentRunnable {
            return 1
        } else {
            return addresses.count
        }
    }
    
    func tableView(_ tableView: NSTableView, objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {
        if currentRunnable {
            return "-"
        }
        
        if tableColumn?.title == "Address" {
            return addresses[row]
        } else if tableColumn?.title == "Instruction" {
            return instructions[row]
        }
        return nil
    }
}
