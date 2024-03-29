=begin
  Copyright (C) 2008 Tobias Brunner
  Hochschule fuer Technik Rapperswil

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
  for more details.

  $Id: guest.rb 4295 2008-08-27 07:35:20Z tobias $
=end

module Dumm
  class Guest
    # accessor for guests
    # e.g. Guest.sun instead of Guest["sun"]
    def self.method_missing(id, *args)
      unless guest? id
        super(id, *args)
      end
      Guest[id]
    end
    
    # accessor for interfaces
    # e.g. guest.eth0 instead of guest["eth0"]
    def method_missing(id, *args)
      unless iface? id
        super(id, *args)
      end
      self[id]
    end
  end
end

# vim:sw=2 ts=2 et
