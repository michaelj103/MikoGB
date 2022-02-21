//
//  InstructionTableCell.swift
//  MikoGB
//
//  Created by Michael Brandt on 2/20/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

import Cocoa


class InstructionTableCell : NSTableCellView {
    override var objectValue: Any? {
        didSet {
            _updateTextField()
        }
    }
    
    private func _updateTextField() {
        self.textField?.objectValue = self.objectValue
    }
}
