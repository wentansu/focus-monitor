// Copyright (c) 2024 Presage Technologies.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation, version 2
// of the License
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

package com.presage.physiology;
import com.presage.physiology.proto.StatusProto;
import com.google.protobuf.MessageLite;

/**
 * Java wrapper class to handle messaging from the Presage Physiology package.
 */

public class Messages {

    public static String getStatusDescription(StatusProto.StatusCode code){
        return nativeGetStatusDescription(code.getNumber());
    }

    public static String getStatusHint(StatusProto.StatusCode code){
        return nativeGetStatusHint(code.getNumber());
    }

    private static native String nativeGetStatusDescription(int code);
    private static native String nativeGetStatusHint(int code);

    private Messages() {}
}

