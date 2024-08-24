/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utility.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: janraub <janraub@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/20 10:36:26 by janraub           #+#    #+#             */
/*   Updated: 2024/08/22 19:24:40 by janraub          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <iostream>

class Utility
{
    public:
        static std::string trimCommentsAndWhitespaces(std::string & line);
        static void printError(std::string const & error);
};
