/*
 * Copyright (C) 2014  Andrew Gunnerson <andrewgunnerson@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

package com.github.chenxiaolong.dualbootpatcher.switcher;

import android.app.Dialog;
import android.app.DialogFragment;
import android.app.Fragment;
import android.os.Bundle;
import android.view.Window;
import android.view.WindowManager;
import android.widget.EditText;
import android.widget.TextView;

import com.afollestad.materialdialogs.MaterialDialog;
import com.afollestad.materialdialogs.MaterialDialog.ButtonCallback;
import com.github.chenxiaolong.dualbootpatcher.R;
import com.github.chenxiaolong.dualbootpatcher.RomUtils.RomInformation;

public class RomNameInputDialog extends DialogFragment {
    public static final String TAG = RomNameInputDialog.class.getSimpleName();

    private static final String ARG_ROM = "rom";

    public interface RomNameInputDialogListener {
        void onRomNameChanged(String newName);
    }

    public static RomNameInputDialog newInstance(Fragment parent, RomInformation info) {
        if (parent != null) {
            if (!(parent instanceof RomNameInputDialogListener)) {
                throw new IllegalStateException(
                        "Parent fragment must implement RomNameInputDialogListener");
            }
        }

        RomNameInputDialog frag = new RomNameInputDialog();
        frag.setTargetFragment(parent, 0);
        Bundle args = new Bundle();
        args.putParcelable(ARG_ROM, info);
        frag.setArguments(args);
        return frag;
    }

    RomNameInputDialogListener getOwner() {
        return (RomNameInputDialogListener) getTargetFragment();
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        RomInformation info = getArguments().getParcelable(ARG_ROM);

        String title = String.format(getString(R.string.rename_rom_title),
                info.getDefaultName());
        String message = String.format(getString(R.string.rename_rom_desc),
                info.getDefaultName());

        Dialog dialog = new MaterialDialog.Builder(getActivity())
                .title(title)
                .customView(R.layout.dialog_textbox, true)
                .positiveText(R.string.ok)
                .negativeText(R.string.cancel)
                .callback(new ButtonCallback() {
                    @Override
                    public void onPositive(MaterialDialog dialog) {
                        RomNameInputDialogListener owner = getOwner();
                        if (owner == null) {
                            return;
                        }

                        EditText et = (EditText) dialog.findViewById(R.id.edittext);
                        String newName = et.getText().toString().trim();

                        if (newName.isEmpty()) {
                            owner.onRomNameChanged(null);
                        } else {
                            owner.onRomNameChanged(newName);
                        }
                    }
                })
                .build();

        TextView tv = (TextView)
                ((MaterialDialog) dialog).getCustomView().findViewById(R.id.message);
        tv.setText(message);

        setCancelable(false);
        dialog.setCanceledOnTouchOutside(false);

        // Show keyboard
        Window window = dialog.getWindow();
        window.setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_VISIBLE);

        return dialog;
    }
}
